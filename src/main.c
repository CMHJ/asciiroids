#include <dlfcn.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <wchar.h>

#include "asciiroids.h"
#include "constants.h"
#include "input.c"
#include "terminal.c"
#include "types.h"

// TODO(CMHJ): replace all uses of stdlib functions like printf with write

typedef struct game_code {
    void* game_lib_handle;
    run_game_loop_function_type* run_game_loop;
} game_code;

static void game_state_init(game_state* state) {
    state->mode = GAME_NEW;

    // get inputs
    // TODO(CMHJ): convert these back to printf and remove wide versions
    wprintf(L"Press the enter key...\n");
    // funnily enough this seems to detect the enter key event from starting the program
    const i8 keyboard_fd = keyboard_detect();
    state->controller_fds[0] = keyboard_fd;
}

static void game_state_deinit(game_state* state) {
    // close inputs
    for (u8 player = 0; player < PLAYERS; ++player) {
        close(state->controller_fds[player]);
    }
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

i32 main(i32 argc, char** argv) {
    // TODO(CMHJ): remove this
    (void)argc;
    char* binary_path = argv[0];

    // assumes game lib is in same dir as binary
    char game_lib_path[MAX_PATH] = {0};
    get_dir_path(game_lib_path, binary_path);
    string_concat(game_lib_path, "/libasciiroids.so");

    setlocale(LC_CTYPE, "");
    terminal_setup();
    // TODO(CMHJ): add trap for Ctrl-C signal to do cleanup so that terminal settings are reset properly

    screen_buffer buffer = {.width = SCREEN_BUFFER_WIDTH, .height = SCREEN_BUFFER_HEIGHT, .data = {0}};
    buffer_set(&buffer, light_shade);

    game_state state = {0};
    game_state_init(&state);

    // init game lib modify time using file_has_changed
    i64 game_lib_last_modify_time = 0;
    file_has_changed(game_lib_path, &game_lib_last_modify_time);

    game_code code = {0};
    code.run_game_loop = run_game_loop_stub;
    game_code_load(&code, game_lib_path);

    while (state.mode != GAME_QUIT) {
        if (file_has_changed(game_lib_path, &game_lib_last_modify_time)) {
            game_code_unload(&code);
            game_code_load(&code, game_lib_path);
        }

        update_inputs(&state);

        code.run_game_loop(&state, &buffer);

        buffer_render(&buffer);

        // TODO(CMHJ): calculate elapsed time in cycle and enforce true 60 FPS
        static const u32 microseconds_in_second = 1000000;
        usleep(microseconds_in_second / FPS);
    }

    game_code_unload(&code);
    game_state_deinit(&state);
    terminal_teardown();

    return EXIT_SUCCESS;
}
