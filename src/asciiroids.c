#include "asciiroids.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#define PI 3.14159265358979323846f
#define DEG_360 360.0f

// player constants
#define YAW_DEG_PER_SEC 180.0f
#define BOOST_ACCELERATION 20.0f

#undef assert
static void exit_with_message(const char* exp, const char* file, const char* func, const i32 line) {
    fprintf(stderr, "Assertion failed: %s (%s: %s: %d)\n", exp, file, func, line);
    fflush(NULL);
    exit(EXIT_FAILURE);
}
#define assert(exp) ((void)((exp) || (exit_with_message(#exp, __FILE__, __func__, __LINE__), 0)))

static void print_xy(screen_buffer* buffer, u32 x, u32 y, wchar_t* string, usize n) {
    // TODO(CMHJ): add wrapping to this function
    wchar_t* dst = &buffer->data[(y * buffer->width) + x];
    wcsncpy(dst, string, n);
}

static void printwc_xy(screen_buffer* buffer, u32 x, u32 y, wchar_t c) {
    static wchar_t buf[2] = {0};
    buf[0] = c;
    print_xy(buffer, x, y, buf, 1);
}

static void buffer_clear(screen_buffer* buffer) {
    for (usize i = 0; i < buffer->width * buffer->height; ++i) {
        buffer->data[i] = NO_SHADE;
    }
}

static inline f32 to_radians(const f32 degrees) {
    return degrees * (PI / 180.0f);
}

static inline f32 v2_mag(const v2 v) {
    return sqrtf((v.x * v.x) + (v.y * v.y));
}

static void update_player_input(player_state* player, controller_state* controller) {
    static const f32 MAX_VEL_MAG = 20.0f;

    // TODO(CMHJ): refactor boost code
    // handle boost
    if (controller->up) {
        v2 vel_new = (v2){player->vel.x + BOOST_ACCELERATION * cosf(to_radians(player->yaw)) / FPS,
                          player->vel.y + BOOST_ACCELERATION * sinf(to_radians(player->yaw)) / FPS};

        const f32 vel_new_mag = v2_mag(vel_new);
        if (vel_new_mag <= MAX_VEL_MAG) {
            player->vel = vel_new;
        } else {
            player->vel.x -= BOOST_ACCELERATION * (player->vel.x / MAX_VEL_MAG) / FPS;
            player->vel.y -= BOOST_ACCELERATION * (player->vel.y / MAX_VEL_MAG) / FPS;
            const f32 vel_subbed_mag = v2_mag(player->vel);

            player->vel.x += BOOST_ACCELERATION * cosf(to_radians(player->yaw)) / FPS;
            player->vel.y += BOOST_ACCELERATION * sinf(to_radians(player->yaw)) / FPS;

            // cap magnitude of velocity to handle float precision errors
            while (v2_mag(player->vel) > MAX_VEL_MAG) {
                player->vel.x -= 0.01f;
                player->vel.y -= 0.01f;
            }
        }
    }

    // handle turning
    static const f32 YAW_TICK = YAW_DEG_PER_SEC / FPS;
    if (controller->left && !controller->right) {
        player->yaw += YAW_TICK;

        // clip yaw to [0, 360.0)
        if (player->yaw >= DEG_360) {
            player->yaw -= DEG_360;
        }
    } else if (controller->right && !controller->left) {
        player->yaw -= YAW_TICK;

        if (player->yaw < 0.0f) {
            player->yaw += DEG_360;
        }
    }

    // handle shooting
    if (controller->shoot) {
        if (player->shot_cooloff_frames == 0) {
            static const u16 shot_cooloff_frames = 5;
            player->shot_cooloff_frames = shot_cooloff_frames;

            // find a bullet slot that has expired
            for (u8 i = 0; i < MAX_BULLETS; ++i) {
                if (player->bullets[i].life_frames == 0) {
                    player->bullets[i].life_frames = 60;
                    player->bullets[i].pos = player->pos;
                    player->bullets[i].yaw = player->yaw;

                    static const f32 bullet_speed = 20.0f;
                    player->bullets[i].vel = (v2){bullet_speed * cosf(to_radians(player->bullets[i].yaw)),
                                                  bullet_speed * sinf(to_radians(player->bullets[i].yaw))};

                    break;
                }
            }
        } else {
            player->shot_cooloff_frames -= 1;
        }
    }
}

static void update_player_pos(screen_buffer* buffer, player_state* player) {
    // account for there being a difference in the height and width of characters.
    // because chars are taller than they are wide, moving north/south is much faster than east/west.
    // this factor accounts for that to make the speed seem smooth
    static const f32 CHAR_SIZE_FACTOR = 2.5f;

    player->pos.x += (player->vel.x * CHAR_SIZE_FACTOR) / FPS;
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

static void update_player_bullets(screen_buffer* buffer, player_state* player) {
    // account for there being a difference in the height and width of characters.
    // because chars are taller than they are wide, moving north/south is much faster than east/west.
    // this factor accounts for that to make the speed seem smooth
    static const f32 CHAR_SIZE_FACTOR = 2.5f;

    for (u8 i = 0; i < MAX_BULLETS; ++i) {
        bullet* b = &(player->bullets[i]);
        if (b->life_frames == 0) {
            continue;
        }

        // tick bullet life down each frame
        b->life_frames -= 1;

        b->pos.x += (b->vel.x * CHAR_SIZE_FACTOR) / FPS;
        if (b->pos.x >= buffer->width) {
            b->pos.x -= buffer->width;

        } else if (b->pos.x < 0.0f) {
            b->pos.x += buffer->width;
        }

        b->pos.y += b->vel.y / FPS;
        if (b->pos.y >= buffer->height)
            b->pos.y -= buffer->height;
        else if (b->pos.y < 0.0f)
            b->pos.y += buffer->height;
    }
}

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

static void render_player_ship(screen_buffer* buffer, player_state* player) {
    const usize x = (usize)player->pos.x;
    const usize y = (buffer->height - 1) - (usize)player->pos.y;
    const usize index = (y * buffer->width) + x;

    assert(index >= 0);
    assert(index < buffer->size);

    static const f32 deg_per_segment = DEG_360 / (f32)TOTAL_DIRECTIONS;
    const f32 deg_half_seg = deg_per_segment / 2.0f;

    // offset yaw by half seg to make math easier, so there's no discontinuity before 0
    f32 yaw = player->yaw + deg_half_seg;
    if (yaw >= DEG_360) {
        yaw -= DEG_360;
    }

    // get the direction segment index based on yaw
    const f32 dir_f = yaw / deg_per_segment;
    const ship_direction dir = dir_f;

    assert(dir >= 0);
    assert(dir < TOTAL_DIRECTIONS);

    buffer->data[index] = SHIP_CHARS[dir];
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
    }

    update_player_input(player1, keyboard_controller_state);
    update_player_pos(buffer, player1);
    update_player_bullets(buffer, player1);

    render_debug_overlay(buffer, player1, keyboard_controller_state);
    render_bullets(buffer, player1);
    render_player_ship(buffer, player1);

    printwc_xy(buffer, 20, 15, BLOCK_UPPER_HALF);
    printwc_xy(buffer, 22, 15, BLOCK_LOWER_HALF);

    printwc_xy(buffer, 30, 15, BLOCK_FULL);
    printwc_xy(buffer, 31, 15, BLOCK_FULL);

    printwc_xy(buffer, 40, 15, BLOCK_FULL);
    printwc_xy(buffer, 41, 15, BLOCK_FULL);
    printwc_xy(buffer, 40, 16, BLOCK_FULL);
    printwc_xy(buffer, 41, 16, BLOCK_FULL);
    printwc_xy(buffer, 42, 15, BLOCK_FULL);
    printwc_xy(buffer, 43, 15, BLOCK_FULL);
    printwc_xy(buffer, 42, 16, BLOCK_FULL);
    printwc_xy(buffer, 43, 16, BLOCK_FULL);

    printwc_xy(buffer, 50, 15, SAUCER_LARGE_LEFT);
    printwc_xy(buffer, 51, 15, SAUCER_LARGE_LEFT_MID);
    printwc_xy(buffer, 52, 15, SAUCER_LARGE_RIGHT_MID);
    printwc_xy(buffer, 53, 15, SAUCER_LARGE_RIGHT);

    printwc_xy(buffer, 60, 15, SAUCER_SMALL_LEFT);
    printwc_xy(buffer, 61, 15, SAUCER_SMALL_RIGHT);

    for (u8 i = 0; i < NUMBERS_NUM; ++i) {
        printwc_xy(buffer, 0 + i, 0, NUMBERS[i]);
    }
}
