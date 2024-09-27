#include "asciiroids.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#define PI 3.14159265358979323846f

// player constants
#define YAW_DEG_PER_SEC 180.0f
#define BOOST_ACCELERATION 20.0f

#undef assert
static void exit_with_message(const char* exp, const char* file, const char* func, const i32 line) {
    fprintf(stderr, "Assertion failed: %s (%s: %s: %d)\n", exp, file, func, line);
    fflush(NULL);
    exit(1);
}
#define assert(exp) ((void)((exp) || (exit_with_message(#exp, __FILE__, __func__, __LINE__), 0)))

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

static void update_player_input(player_state* player, controller_state* controller) {
    if (controller->up) {
        player->vel.x += BOOST_ACCELERATION * cosf(to_radians(player->yaw)) / FPS;
        player->vel.y += BOOST_ACCELERATION * sinf(to_radians(player->yaw)) / FPS;
    }

    static const f32 YAW_TICK = YAW_DEG_PER_SEC / FPS;
    if (controller->left && !controller->right) {
        player->yaw += YAW_TICK;

        // clip yaw to [0, 360.0)
        if (player->yaw >= 360.0f) {
            player->yaw -= 360.0f;
        }
    } else if (controller->right && !controller->left) {
        player->yaw -= YAW_TICK;

        if (player->yaw < 0.0f) {
            player->yaw += 360.0f;
        }
    }
}

static void update_player_pos(screen_buffer* buffer, player_state* player) {
    player->pos.x += player->vel.x / FPS;
    if (player->pos.x >= buffer->width) {
        player->pos.x -= buffer->width;

    } else if (player->pos.x < 0.0f) {
        player->pos.x += buffer->width;
    }

    player->pos.y += player->vel.y / FPS;
    if (player->pos.y >= buffer->height)
        player->pos.y -= buffer->height;
    else if (player->pos.y < 0.0f)
        player->pos.y += buffer->height;
}

static void render_debug_overlay(screen_buffer* buffer, player_state* player, controller_state* controller) {
    wchar_t msg_buf[100] = {0};

    swprintf(msg_buf, sizeof(msg_buf), L"Yaw: %.2f", player->yaw);
    print_xy(buffer, 0, 21, msg_buf, wcslen(msg_buf));
    swprintf(msg_buf, sizeof(msg_buf), L"X: %.2f, Y: %.2f", player->pos.x, player->pos.y);
    print_xy(buffer, 0, 22, msg_buf, wcslen(msg_buf));
    swprintf(msg_buf, sizeof(msg_buf), L"VelX: %.2f, VelY: %.2f", player->vel.x, player->vel.y);
    print_xy(buffer, 0, 23, msg_buf, wcslen(msg_buf));

    // visualise keyboard input
    buffer->data[buffer->size - 4] = controller->shoot ? dark_shade : light_shade;
    buffer->data[buffer->size - 3] = controller->left ? dark_shade : light_shade;
    buffer->data[buffer->size - 2] = controller->up ? dark_shade : light_shade;
    buffer->data[buffer->size - 1] = controller->right ? dark_shade : light_shade;
}

static void render_player_ship(screen_buffer* buffer, player_state* player) {
    const usize x = (usize)player->pos.x;
    const usize y = (buffer->height - 1) - (usize)player->pos.y;
    const usize index = (y * buffer->width) + x;

    assert(index >= 0);
    assert(index < buffer->size);

    buffer->data[index] = dark_shade;
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
    }

    update_player_input(player1, keyboard_controller_state);
    update_player_pos(buffer, player1);

    render_debug_overlay(buffer, player1, keyboard_controller_state);

    // TODO(CMHJ): fix ship not rendering on the top and right side
    render_player_ship(buffer, player1);
}
