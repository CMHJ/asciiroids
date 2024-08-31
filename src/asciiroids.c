#include "asciiroids.h"

#include <stdio.h>

RUN_GAME_LOOP(run_game_loop) {
    controller_state* keyboard_controller_state = &state->controllers[0];

    if (keyboard_controller_state->quit) {
        state->running = false;
    }

    // visualise keyboard input
    buffer->data[1] = keyboard_controller_state->left ? dark_shade : light_shade;
    buffer->data[2] = keyboard_controller_state->up ? dark_shade : light_shade;
    buffer->data[3] = keyboard_controller_state->right ? dark_shade : light_shade;
    buffer->data[0] = keyboard_controller_state->shoot ? dark_shade : light_shade;

    // run either line and then recompile to see that it gets dynamically loaded
    for (usize i = 5; i < buffer->width * buffer->height; ++i) {
        // if (i % 2) buffer->data[i] = dark_shade;

        buffer->data[i] = light_shade;
    }
}
