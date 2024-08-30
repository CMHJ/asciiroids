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

int32_t main(void) {
    setlocale(LC_CTYPE, "");
    terminal_setup();

    screen_buffer buffer = {.width = SCREEN_BUFFER_WIDTH, .height = SCREEN_BUFFER_HEIGHT, .data = {0}};
    buffer_set(&buffer, light_shade);

    game_state state = {0};
    game_state_init(&state);

    while (state.running) {
        update_inputs(&state);

        run_game_loop(&state, &buffer);

        buffer_render(&buffer);

        // TODO(CMHJ): calculate elapsed time in cycle and enforce true 60 FPS
        static const u32 microseconds_in_second = 1000000;
        static const u32 fps = 60;
        usleep(microseconds_in_second / fps);
    }

    game_state_deinit(&state);
    terminal_teardown();

    return EXIT_SUCCESS;
}
