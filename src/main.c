#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "input_keyboard.c"
#include "terminal.c"
#include "types.h"

#define WIDTH 80
#define HEIGHT 24
#define FPS 60

int32_t main(void) {
    setlocale(LC_CTYPE, "");
    terminal_setup();

    wchar_t screen_buffer[HEIGHT * WIDTH] = {0};
    static const wchar_t light_shade = L'\u2591';
    static const wchar_t dark_shade = L'\u2593';
    buffer_set(screen_buffer, HEIGHT, WIDTH, light_shade);

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
        screen_buffer[1] = keyboard_controller_state->left ? dark_shade : light_shade;
        screen_buffer[2] = keyboard_controller_state->up ? dark_shade : light_shade;
        screen_buffer[3] = keyboard_controller_state->right ? dark_shade : light_shade;
        screen_buffer[0] = keyboard_controller_state->shoot ? dark_shade : light_shade;

        buffer_render(screen_buffer, HEIGHT, WIDTH);

        // TODO(CMHJ): calculate elapsed time in cycle and enforce true 60 FPS
        static const u32 microseconds_in_second = 1000000;
        usleep(microseconds_in_second / FPS);
    }

    close(keyboard_fd);

    terminal_teardown();

    return EXIT_SUCCESS;
}
