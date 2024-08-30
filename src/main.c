#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "input.c"
#include "terminal.c"
#include "types.h"

static const wchar_t light_shade = L'\u2591';
static const wchar_t dark_shade = L'\u2593';

int32_t main(void) {
    setlocale(LC_CTYPE, "");
    terminal_setup();

    screen_buffer buffer = {.width = SCREEN_BUFFER_WIDTH, .height = SCREEN_BUFFER_HEIGHT, .data = {0}};
    buffer_set(&buffer, light_shade);

    game_state game_state_data = {0};
    controller_state* keyboard_controller_state = &game_state_data.controllers[0];

    // TODO(CMHJ): convert these back to printf and remove wide versions
    wprintf(L"Press the enter key...\n");
    // funnily enough this seems to detect the enter key event from starting the program
    const i8 keyboard_fd = keyboard_detect();

    u32 i = 0;
    for (bool running = true; running; ++i) {
        keyboard_state_update(keyboard_fd, keyboard_controller_state);

        if (keyboard_controller_state->quit) {
            running = false;
        }

        // visualise keyboard input
        buffer.data[1] = keyboard_controller_state->left ? dark_shade : light_shade;
        buffer.data[2] = keyboard_controller_state->up ? dark_shade : light_shade;
        buffer.data[3] = keyboard_controller_state->right ? dark_shade : light_shade;
        buffer.data[0] = keyboard_controller_state->shoot ? dark_shade : light_shade;

        buffer_render(&buffer);

        // TODO(CMHJ): calculate elapsed time in cycle and enforce true 60 FPS
        static const u32 microseconds_in_second = 1000000;
        static const u32 fps = 60;
        usleep(microseconds_in_second / fps);
    }

    close(keyboard_fd);
    terminal_teardown();

    return EXIT_SUCCESS;
}
