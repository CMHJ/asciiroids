#include "asciiroids.h"

#include <math.h>
#include <stdio.h>
#include <wchar.h>

static void print_xy(screen_buffer* buffer, u32 x, u32 y, wchar_t* string, usize n) {
    wchar_t* dst = &buffer->data[(y * buffer->width) + x];
    wcsncpy(dst, string, n);
}

static void buffer_clear(screen_buffer* buffer) {
    for (usize i = 0; i < buffer->width * buffer->height; ++i) {
        buffer->data[i] = light_shade;
    }
}

RUN_GAME_LOOP(run_game_loop) {
    wchar_t msg_buf[100] = {0};

    controller_state* keyboard_controller_state = &state->controllers[0];
    player_state* player1 = &state->players[0];

    buffer_clear(buffer);

    if (keyboard_controller_state->quit) {
        state->running = false;
    }

    static bool game_start = true;
    if (game_start) {
        game_start = false;

        player1->pos = (v2){SCREEN_BUFFER_HEIGHT / 2.0f, SCREEN_BUFFER_WIDTH / 2.0f};
        player1->yaw = 90.0f;
    }

    if (keyboard_controller_state->left) {
        player1->yaw += 1.0f;
        if (player1->yaw >= 360.0f) player1->yaw -= 360.0f;
    } else if (keyboard_controller_state->right) {
        player1->yaw -= 1.0f;
        if (player1->yaw < 0.0f) player1->yaw += 360.0f;
    }
    swprintf(msg_buf, sizeof(msg_buf), L"Yaw: %.2f", player1->yaw);
    print_xy(buffer, 0, 1, msg_buf, wcslen(msg_buf));

    // visualise keyboard input
    buffer->data[1] = keyboard_controller_state->left ? dark_shade : light_shade;
    buffer->data[2] = keyboard_controller_state->up ? dark_shade : light_shade;
    buffer->data[3] = keyboard_controller_state->right ? dark_shade : light_shade;
    buffer->data[0] = keyboard_controller_state->shoot ? dark_shade : light_shade;

    // render ship onto screen
    buffer->data[(usize)((player1->pos.x * buffer->width) + player1->pos.y)] = dark_shade;
}
