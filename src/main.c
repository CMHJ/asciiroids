#include <dlfcn.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "asciiroids.h"
#include "input.c"
#include "terminal.c"
#include "types.h"

static void game_state_init(game_state* state) {
    state->running = true;
    state->game_lib_handle = NULL;

    // get inputs
    // TODO(CMHJ): convert these back to printf and remove wide versions
    wprintf(L"Press the enter key...\n");
    // funnily enough this seems to detect the enter key event from starting the program
    const i8 keyboard_fd = keyboard_detect();
    state->controller_fds[0] = keyboard_fd;
}

static void game_state_deinit(game_state* state) {
    for (u8 player = 0; player < PLAYERS; ++player) {
        close(state->controller_fds[player]);
    }
}

static void game_code_load(game_state* state, char* binary_path) {
    char buf[MAX_PATH] = {0};

    // get pointer to one past last character of directory path
    usize size_of_binary_path = strlen(binary_path);
    char* last_dir_slash = binary_path + size_of_binary_path;
    for (char* scan = binary_path; *scan; ++scan) {
        if (*scan == '/') {
            last_dir_slash = scan;
        }
    }

    // copy to path to buf
    char* dst = buf;
    char* src = binary_path;
    for (; src != last_dir_slash; ++dst, ++src) {
        *dst = *src;
    }

    // append lib name to buf
    char* libname = "/libasciiroids.so";
    src = libname;
    for (; *src != '\0'; ++dst, ++src) {
        *dst = *src;
    }

    // load game code
    state->game_lib_handle = dlopen(buf, RTLD_LAZY);
}

static void game_code_unload(game_state* state) {
    if (state->game_lib_handle != NULL) {
        dlclose(state->game_lib_handle);
    }
}

i32 main(i32 argc, char** argv) {
    // TODO(CMHJ): remove this
    (void)argc;
    char* binary_path = argv[0];

    setlocale(LC_CTYPE, "");
    terminal_setup();

    screen_buffer buffer = {.width = SCREEN_BUFFER_WIDTH, .height = SCREEN_BUFFER_HEIGHT, .data = {0}};
    buffer_set(&buffer, light_shade);

    game_state state = {0};
    game_state_init(&state);
    game_code_load(&state, binary_path);

    while (state.running) {
        update_inputs(&state);

        run_game_loop(&state, &buffer);

        buffer_render(&buffer);

        // TODO(CMHJ): calculate elapsed time in cycle and enforce true 60 FPS
        static const u32 microseconds_in_second = 1000000;
        static const u32 fps = 60;
        usleep(microseconds_in_second / fps);
    }

    game_code_unload(&state);
    game_state_deinit(&state);
    terminal_teardown();

    return EXIT_SUCCESS;
}
