#include "asciiroids.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "constants.h"
#include "print.h"
#include "utility.h"

#include "assert.c"
#include "enemy.c"
#include "player.c"
#include "print.c"
#include "utility.c"

static void render_debug_overlay(screen_buffer* buffer, player_state* player, controller_state* controller) {
    wchar_t msg_buf[100] = {0};

    swprintf(msg_buf, sizeof(msg_buf), L"Yaw: %.2f", player->yaw);
    print_xy(buffer, 0, 21, msg_buf, wcslen(msg_buf));
    swprintf(msg_buf, sizeof(msg_buf), L"X: %.2f, Y: %.2f", player->pos.x, player->pos.y);
    print_xy(buffer, 0, 22, msg_buf, wcslen(msg_buf));
    swprintf(msg_buf, sizeof(msg_buf), L"VelX: %.2f, VelY: %.2f Mag: %.2f", player->vel.x, player->vel.y,
             v2_mag(player->vel));
    print_xy(buffer, 0, 23, msg_buf, wcslen(msg_buf));

    // visualise keyboard input
    buffer->data[buffer->size - 4] = controller->shoot ? DARK_SHADE : LIGHT_SHADE;
    buffer->data[buffer->size - 3] = controller->left ? DARK_SHADE : LIGHT_SHADE;
    buffer->data[buffer->size - 2] = controller->up ? DARK_SHADE : LIGHT_SHADE;
    buffer->data[buffer->size - 1] = controller->right ? DARK_SHADE : LIGHT_SHADE;
}

static void render_bullets(screen_buffer* buffer, player_state* player) {
    for (u8 i = 0; i < MAX_BULLETS; ++i) {
        if (player->bullets[i].life_frames > 0) {
            const usize x = (usize)player->bullets[i].pos.x;
            const usize y = (buffer->height - 1) - (usize)player->bullets[i].pos.y;
            const usize index = (y * buffer->width) + x;
            buffer->data[index] = U_BULLET;
        }
    }
}

RUN_GAME_LOOP(run_game_loop) {
    controller_state* keyboard_controller_state = &state->controllers[0];
    player_state* player1 = &state->players[0];

    buffer_clear(buffer);

    if (keyboard_controller_state->quit) {
        state->mode = GAME_QUIT;
    }

    if (state->mode == GAME_NEW) {
        state->mode = GAME_RUNNING;

        player1->pos = (v2){buffer->width / 2.0f, buffer->height / 2.0f};
        player1->yaw = 90.0f;

        const u8 asteroids_num = 4;
        for (u8 i = 0; i < asteroids_num; ++i) {
            state->enemies[i].type = ASTEROID_LARGE;
            const f32 angle_offset = -30.0f;
            const f32 angle = (i * DEG_360 / asteroids_num);
            const f32 dist_from_centre = buffer->width / 4.0f;
            const f32 asteroid_vel_mag = 5.0f;
            state->enemies[i].pos = (v2){dist_from_centre * cosf(to_radians(angle + angle_offset)),
                                         dist_from_centre * sinf(to_radians(angle + angle_offset))};
            state->enemies[i].vel = (v2){asteroid_vel_mag * cosf(to_radians(angle + angle_offset * i)),
                                         asteroid_vel_mag * sinf(to_radians(angle + angle_offset * i))};
            state->enemies[i].yaw = i * 150.0f;
        }
    }

    update_player_input(player1, keyboard_controller_state);
    update_player_pos(buffer, player1);
    update_player_bullets(buffer, player1);

    render_debug_overlay(buffer, player1, keyboard_controller_state);
    render_bullets(buffer, player1);
    render_player_ship(buffer, player1);

    update_enemies(buffer, state->enemies);
    render_enemies(buffer, state->enemies);
}
