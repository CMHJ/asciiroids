#include "asciiroids.h"

#include <math.h>
#include <stdio.h>
#include <wchar.h>

#define PI 3.14159265358979323846f

static void print_xy(screen_buffer* buffer, u32 x, u32 y, wchar_t* string, usize n) {
    wchar_t* dst = &buffer->data[(y * buffer->width) + x];
    wcsncpy(dst, string, n);
}

static void buffer_clear(screen_buffer* buffer) {
    for (usize i = 0; i < buffer->width * buffer->height; ++i) {
        buffer->data[i] = light_shade;
    }
}

static f32 to_radians(const f32 degrees) {
    return degrees * (PI / 180.0f);
}

RUN_GAME_LOOP(run_game_loop) {
    wchar_t msg_buf[100] = {0};

    controller_state* keyboard_controller_state = &state->controllers[0];
    player_state* player1 = &state->players[0];

    buffer_clear(buffer);

    if (keyboard_controller_state->quit) {
        state->mode = GAME_QUIT;
    }

    if (state->mode == GAME_NEW) {
        state->mode = GAME_RUNNING;

        player1->pos = (v2){SCREEN_BUFFER_WIDTH / 2.0f, SCREEN_BUFFER_HEIGHT / 2.0f};
        player1->yaw = 90.0f;
    }

    if (keyboard_controller_state->up) {
        player1->vel.x += 5.0f * cosf(to_radians(player1->yaw)) / FPS;
        player1->vel.y += 5.0f * sinf(to_radians(player1->yaw)) / FPS;
    }
    player1->pos.x += player1->vel.x / FPS;
    if (player1->pos.x >= SCREEN_BUFFER_WIDTH)
        player1->pos.x -= SCREEN_BUFFER_WIDTH;
    else if (player1->pos.x < 0.0f)
        player1->pos.x += SCREEN_BUFFER_WIDTH;

    player1->pos.y += player1->vel.y / FPS;
    if (player1->pos.y >= SCREEN_BUFFER_HEIGHT)
        player1->pos.y -= SCREEN_BUFFER_HEIGHT;
    else if (player1->pos.y < 0.0f)
        player1->pos.y += SCREEN_BUFFER_HEIGHT;

    if (keyboard_controller_state->left && !keyboard_controller_state->right) {
        player1->yaw += 1.0f;
        if (player1->yaw >= 360.0f) player1->yaw -= 360.0f;
    } else if (keyboard_controller_state->right && !keyboard_controller_state->left) {
        player1->yaw -= 1.0f;
        if (player1->yaw < 0.0f) player1->yaw += 360.0f;
    }
    swprintf(msg_buf, sizeof(msg_buf), L"Yaw: %.2f", player1->yaw);
    print_xy(buffer, 0, 21, msg_buf, wcslen(msg_buf));
    swprintf(msg_buf, sizeof(msg_buf), L"X: %.2f, Y: %.2f", player1->pos.x, player1->pos.y);
    print_xy(buffer, 0, 22, msg_buf, wcslen(msg_buf));
    swprintf(msg_buf, sizeof(msg_buf), L"VelX: %.2f, VelY: %.2f", player1->vel.x, player1->vel.y);
    print_xy(buffer, 0, 23, msg_buf, wcslen(msg_buf));

    // visualise keyboard input
    buffer->data[1] = keyboard_controller_state->left ? dark_shade : light_shade;
    buffer->data[2] = keyboard_controller_state->up ? dark_shade : light_shade;
    buffer->data[3] = keyboard_controller_state->right ? dark_shade : light_shade;
    buffer->data[0] = keyboard_controller_state->shoot ? dark_shade : light_shade;

    // TODO(CMHJ): fix ship not rendering on the top and right side

    // render ship onto screen
    usize index = SCREEN_BUFFER_HEIGHT - player1->pos.y;
    buffer->data[((SCREEN_BUFFER_HEIGHT - (usize)player1->pos.y) * buffer->width) + (usize)player1->pos.x] = dark_shade;
}
