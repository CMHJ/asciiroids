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

static void game_init(game_state* state, screen_buffer* buffer) {
    state->mode = GAME_RUNNING;

    state->players[0].phy.pos = (v2){buffer->width / 2.0f, buffer->height / 2.0f};
    state->players[0].phy.yaw = 90.0f;

    const u8 asteroids_num = 4;
    for (u8 i = 0; i < asteroids_num; ++i) {
        state->enemies[i].type = ASTEROID_LARGE;
        const f32 pos_angle_from_origin = rand() % 360;
        const f32 yaw = rand() % 360;
        const f32 dist_from_centre = buffer->width / 4.0f;
        const f32 vel_mag = VEL_MAX_ASTEROID_LARGE * (rand() / (f32)RAND_MAX);
        state->enemies[i].phy.pos = (v2){dist_from_centre * cosf(to_radians(pos_angle_from_origin)),
                                         dist_from_centre * sinf(to_radians(pos_angle_from_origin))};
        state->enemies[i].phy.vel = (v2){vel_mag * cosf(to_radians(yaw)), vel_mag * sinf(to_radians(yaw))};
    }
}

static void render_debug_overlay(screen_buffer* buffer, player_state* player, controller_state* controller) {
    wchar_t msg_buf[100] = {0};

    swprintf(msg_buf, sizeof(msg_buf), L"Yaw: %.2f", player->phy.yaw);
    print_xy(buffer, 0, 21, msg_buf, wcslen(msg_buf));
    swprintf(msg_buf, sizeof(msg_buf), L"X: %.2f, Y: %.2f", player->phy.pos.x, player->phy.pos.y);
    print_xy(buffer, 0, 22, msg_buf, wcslen(msg_buf));
    swprintf(msg_buf, sizeof(msg_buf), L"VelX: %.2f, VelY: %.2f Mag: %.2f", player->phy.vel.x, player->phy.vel.y,
             v2_mag(player->phy.vel));
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
            const usize x = (usize)player->bullets[i].phy.pos.x;
            const usize y = (buffer->height - 1) - (usize)player->bullets[i].phy.pos.y;
            const usize index = (y * buffer->width) + x;
            buffer->data[index] = U_BULLET;
        }
    }
}

static void update_position(screen_buffer* buffer, physics* phy) {
    // account for there being a difference in the height and width of characters.
    // because chars are taller than they are wide, moving north/south is much faster than east/west.
    // this factor accounts for that to make the speed seem smooth
    static const f32 CHAR_SIZE_FACTOR = 2.5f;

    phy->pos.x += (phy->vel.x * CHAR_SIZE_FACTOR) / FPS;
    if (phy->pos.x >= buffer->width) {
        phy->pos.x -= buffer->width;

    } else if (phy->pos.x < 0.0f) {
        phy->pos.x += buffer->width;
    }

    phy->pos.y += phy->vel.y / FPS;
    if (phy->pos.y >= buffer->height)
        phy->pos.y -= buffer->height;
    else if (phy->pos.y < 0.0f)
        phy->pos.y += buffer->height;
}

static bool asteroid_collision(v2 asteroid_pos, v2 object_pos, f32 asteroid_width, f32 asteroid_height) {
    return (asteroid_pos.x <= object_pos.x && asteroid_pos.x + asteroid_width >= object_pos.x &&
            asteroid_pos.y <= object_pos.y && asteroid_pos.y + asteroid_height >= object_pos.y);
}

static enemy_state* get_dead_enemy(game_state* game) {
    for (u8 i = 0; i < MAX_ENEMIES; ++i) {
        enemy_state* enemy = &(game->enemies[i]);
        if (enemy->type == DEAD) {
            return enemy;
        }
    }
}

static void enemy_asteroid_init(enemy_type type) {
}

static void update_enemies(screen_buffer* buffer, game_state* game) {
    for (u8 i = 0; i < MAX_ENEMIES; ++i) {
        enemy_state* e = &(game->enemies[i]);
        if (e->type == DEAD) {
            continue;
        }

        update_position(buffer, &e->phy);

        switch (e->type) {
            case ASTEROID_SMALL: {
                break;
            }
            case ASTEROID_MEDIUM: {
                break;
            }
            case ASTEROID_LARGE: {
                // check if any player's bullets have collided
                for (u8 j = 0; j < PLAYERS; ++j) {
                    for (u8 k = 0; k < MAX_BULLETS; ++k) {
                        bullet* b = &game->players[j].bullets[k];
                        if (b->life_frames == 0) {
                            continue;
                        }

                        if (asteroid_collision(e->phy.pos, b->phy.pos, 4.0f, 2.0f)) {
                            b->life_frames = 0;
                            e->type = ASTEROID_MEDIUM;

                            const f32 vel_mag = VEL_MAX_ASTEROID_MEDIUM * (rand() / (f32)RAND_MAX);
                            const f32 yaw = get_random_angle();
                            e->phy.vel.x = vel_mag * deg_cos(yaw);
                            e->phy.vel.y = vel_mag * deg_sin(yaw);

                            // look for a spot to stick the other spawned asteroid
                            enemy_state* new_enemy = get_dead_enemy(game);
                            new_enemy->type = ASTEROID_MEDIUM;
                            new_enemy->phy.pos = e->phy.pos;

                            const f32 vel_mag_2 = VEL_MAX_ASTEROID_MEDIUM * (rand() / (f32)RAND_MAX);
                            const f32 yaw_2 = get_random_angle();
                            new_enemy->phy.vel.x = vel_mag_2 * deg_cos(yaw_2);
                            new_enemy->phy.vel.y = vel_mag_2 * deg_sin(yaw_2);
                        }
                    }
                }

                break;
            }
            case SAUCER_SMALL: {
                break;
            }
            case SAUCER_LARGE: {
                break;
            }
            case DEAD:
            default: {
                fprintf(stderr, "Error: unhandled case %d\n", (u32)e->type);
                exit(1);
            }
        }
    }
}

static void update_bullets(screen_buffer* buffer, bullet* bullets) {
    // TODO(CMHJ): make n bullets part of struct
    for (u8 i = 0; i < MAX_BULLETS; ++i) {
        bullet* b = &bullets[i];
        if (b->life_frames == 0) {
            continue;
        }

        // tick bullet life down each frame
        b->life_frames -= 1;

        update_position(buffer, &b->phy);
    }
}

RUN_GAME_LOOP(run_game_loop) {
    controller_state* keyboard_controller_state = &game->controllers[0];
    player_state* player1 = &game->players[0];

    buffer_clear(buffer);

    if (keyboard_controller_state->quit) {
        game->mode = GAME_QUIT;
    }

    if (game->mode == GAME_NEW) {
        game_init(game, buffer);
    }

    update_player_input(player1, keyboard_controller_state);

    update_position(buffer, &player1->phy);
    update_bullets(buffer, player1->bullets);
    update_enemies(buffer, game);

    render_debug_overlay(buffer, player1, keyboard_controller_state);
    render_bullets(buffer, player1);
    render_enemies(buffer, game->enemies);
    render_player_ship(buffer, player1);

    for (u8 i = 0; i < buffer->width; i++) {
        wchar_t c = L'0' + (i % 10);
        printwc_xy(buffer, i, buffer->height - 4, c);
    }
}
