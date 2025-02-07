#define _POSIX_C_SOURCE 199309L  // clock_gettime, nanosleep

#include <dlfcn.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <wchar.h>
#include <signal.h>

#include "asciiroids.h"
#include "constants.h"
#include "types.h"

#include "input.c"
#include "terminal.c"

#ifdef RELEASE
#include "asciiroids.c"
#endif

// TODO(CMHJ): replace all uses of stdlib functions like printf with write

typedef struct game_code {
    void* game_lib_handle;
    run_game_loop_function_type* run_game_loop;
} game_code;

static void controllers_init(i8* controller_fds) {
    // get inputs
    // TODO(CMHJ): convert these back to printf and remove wide versions
    wprintf(L"Detecting keyboard\n");
    wprintf(L"Press the right ctrl key...\n");
    // funnily enough this seems to detect the enter key event from starting the program
    const i8 keyboard_fd = keyboard_detect();

    // assume all players use the keyboard
    for (u8 i = 0; i < PLAYERS_MAX; ++i) {
        controller_fds[i] = keyboard_fd;
    }
}

static void controllers_deinit(i8* controller_fds) {
    // close inputs
    for (u8 player = 0; player < PLAYERS_MAX; ++player) {
        close(controller_fds[player]);
    }
}

static u64 get_timestamp_ns(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
        perror("clock_gettime");
        exit(1);
    }

    u64 secs_in_nsecs = ts.tv_sec * 1000 * 1000 * 1000;
    u64 result = secs_in_nsecs + (u64)ts.tv_nsec;
    return result;
}

static void sleep_ns(u64 ns) {
    struct timespec duration;
    duration.tv_nsec = ns % (1000 * 1000 * 1000);
    duration.tv_sec = ns / (1000 * 1000 * 1000);

    if (nanosleep(&duration, NULL) == -1) {
        perror("nanosleep");
        exit(1);
    }

    return;
}

static void get_dir_path(char* buf, char* binary_path) {
    // TODO(CMHJ): don't use strlen
    usize size_of_binary_path = strlen(binary_path);

    // get pointer to last character of directory path
    char* last_dir_slash = binary_path + size_of_binary_path;
    for (char* scan = binary_path; *scan; ++scan) {
        if (*scan == '/') {
            last_dir_slash = scan;
        }
    }

    // copy to path to buf
    for (char *dst = buf, *src = binary_path; src != last_dir_slash; ++dst, ++src) {
        *dst = *src;
    }
}

static void string_concat(char* dst, char* src) {
    // advance dst to null terminator
    while (*dst != '\0') {
        ++dst;
    }

    // append src to dst
    for (; *src != '\0'; ++dst, ++src) {
        *dst = *src;
    }
}

static bool file_has_changed(char* file_path, i64* last_modify_time) {
    bool changed = false;
    struct stat file_stat;

    if (stat(file_path, &file_stat) != 0) {
        // TODO(CMHJ): log error? ignore and carry on if file isn't ready
        changed = false;
        return changed;
    }

    if (*last_modify_time != (i64)file_stat.st_mtime) {
        *last_modify_time = (i64)file_stat.st_mtime;
        changed = true;
    }

    return changed;
}

static void game_code_load(game_code* code, char* lib_path) {
    // spin until library is available
    while (code->game_lib_handle == NULL) {
        code->game_lib_handle = dlopen(lib_path, RTLD_LAZY);
    }

    // cast to intptr is to shut gcc up about the conversion of an void* object pointer to a function pointer
    code->run_game_loop = (run_game_loop_function_type*)(intptr_t)dlsym(code->game_lib_handle, "run_game_loop");
    if (code->run_game_loop == NULL) {
        fprintf(stderr, "Error loading function: %s\n", dlerror());
        dlclose(code->game_lib_handle);
        exit(EXIT_FAILURE);
    }
}

static void game_code_unload(game_code* code) {
    if (code->game_lib_handle != NULL) {
        dlclose(code->game_lib_handle);
        code->game_lib_handle = NULL;
        code->run_game_loop = run_game_loop_stub;
    }
}

void cleanup(int sys_signal) {
    (void)sys_signal;  // unused
    system("reset");   // reset terminal settings, terminal_teardown doesn't work and just hangs for some reason
    exit(0);
}

i32 main(i32 argc, char** argv) {
    // TODO(CMHJ): remove this
    (void)argc;
    char* binary_path = argv[0];

#ifndef RELEASE
    // assumes game lib is in same dir as binary
    char game_lib_path[MAX_PATH] = {0};
    get_dir_path(game_lib_path, binary_path);
    string_concat(game_lib_path, "/libasciiroids.so");
#endif

    srand(0);  // have the same seed each time
    setlocale(LC_CTYPE, "");
    terminal_setup();

    // restore terminal settings if killed
    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);
    signal(SIGKILL, cleanup);

    screen_buffer buffer = {.width = SCREEN_BUFFER_WIDTH,
                            .height = SCREEN_BUFFER_HEIGHT,
                            .size = SCREEN_BUFFER_WIDTH * SCREEN_BUFFER_HEIGHT,
                            .data = {0}};
    buffer_set(&buffer, LIGHT_SHADE);

    game_state state = {0};
    state.mode = GAME_MAIN_MENU;

    i8 controller_fds[PLAYERS_MAX] = {0};
    controllers_init(controller_fds);

#ifndef RELEASE
    // init game lib modify time using file_has_changed
    i64 game_lib_last_modify_time = 0;
    file_has_changed(game_lib_path, &game_lib_last_modify_time);
#endif

    game_code code = {0};

#ifndef RELEASE
    code.run_game_loop = run_game_loop_stub;
    game_code_load(&code, game_lib_path);
#else
    code.run_game_loop = run_game_loop;
#endif

    u64 ts_frame_start = 0;
    u64 ts_cycle_start = 0;
    u64 ts_end = 0;
    u64 frame_elapsed_ns = 0;
    u64 cycle_elapsed_ns = 0;
    while (state.mode != GAME_QUIT) {
        ts_frame_start = get_timestamp_ns();

#ifndef RELEASE
        if (file_has_changed(game_lib_path, &game_lib_last_modify_time)) {
            game_code_unload(&code);
            game_code_load(&code, game_lib_path);
        }
#endif

        update_inputs(&state, controller_fds);

        code.run_game_loop(&state, &buffer);

#ifndef RELEASE
        swprintf(buffer.data + buffer.size - buffer.width, buffer.size, L"frame: %f ms, cycle: %f ms",
                 (f32)frame_elapsed_ns / 1e6, (f32)cycle_elapsed_ns / 1e6);
#endif

        buffer_render(&buffer);

        ts_end = get_timestamp_ns();
        frame_elapsed_ns = ts_end - ts_frame_start;
        cycle_elapsed_ns = ts_end - ts_cycle_start;
        ts_cycle_start = get_timestamp_ns();

        static const f32 nanoseconds_in_second = 1000 * 1000 * 1000;
        const f32 cycle_period_ns = nanoseconds_in_second / FPS;
        const f32 remaining_time_in_cycle_ns = cycle_period_ns - frame_elapsed_ns;
        if (remaining_time_in_cycle_ns > 0) {
            sleep_ns(remaining_time_in_cycle_ns);
        }
    }

#ifndef RELEASE
    game_code_unload(&code);
#endif

    controllers_deinit(controller_fds);

    // programs exits here and terminal_teardown is not called
    cleanup(0);

    // TODO: make this cleanly exit instead of relying on lazy call to reset
    terminal_teardown();
}
