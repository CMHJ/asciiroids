#include "asciiroids.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <string.h>

#include "constants.h"
#include "print.h"
#include "utility.h"

#include "assert.c"
#include "enemy.c"
#include "player.c"
#include "print.c"
#include "utility.c"

static void render_ui(screen_buffer* buffer, game_state* game) {
    for (u8 p_i = 0; p_i < PLAYERS; ++p_i) {
        player_state* player = &game->players[p_i];

        static const usize margin = 2;
        const usize spacing = (buffer->width - 2 * margin) / 4;
        const usize x_offset = margin + (p_i + 1) * spacing;

        // draw level counter
        wchar_t msg_buf[U32_MAX_DIGITS + 1] = {0};
        swprintf(msg_buf, sizeof(msg_buf), L"%02d", game->level);
        print_xy(buffer, buffer->width / 2, buffer->height - 1, msg_buf, wcslen(msg_buf));

        // draw score
        swprintf(msg_buf, sizeof(msg_buf), L"%d", player->score);
        print_xy(buffer, x_offset, buffer->height - 2, msg_buf, wcslen(msg_buf));

        // draw lives
        for (u8 life = 0; life < player->lives; ++life) {
            printwc_xy(buffer, x_offset - life, buffer->height - 3, U_SHIP_N);
        }
    }
}

static void player_kill(player_state* player) {
    player->alive = false;
    player->respawn_frames = 60;
}

static void enemy_asteroid_init(enemy_state* e, enemy_type type, v2 pos) {
    e->type = type;
    e->phy.pos = pos;

    const f32 vel_mag = ENEMY_MAX_VEL[type] * (rand() / (f32)RAND_MAX);
    const f32 yaw = get_random_angle();
    e->phy.vel.x = vel_mag * deg_cos(yaw);
    e->phy.vel.y = vel_mag * deg_sin(yaw);
}

static void enemy_saucer_init(screen_buffer* buffer, enemy_state* e) {
    // randomly pick left or right
    const bool going_right = rand() % 2;

    // spawn saucer at random height at the edge of the screen
    e->phy.pos = (v2){going_right ? 0.0f : buffer->width - 1, buffer->height * (rand() / (f32)RAND_MAX)};
    e->direction_change_frames = 0;
    e->saucer_bullet.life_frames = 0;

    const f32 vel_mag = ENEMY_MAX_VEL[e->type];
    // pick a direction in a 90 degree cone centred on 0
    e->phy.yaw = (rand() % 90) - 45;
    // adjust angle for direction
    if (going_right == false) e->phy.yaw += 180.0f;

    e->phy.yaw = degrees_clip(e->phy.yaw);

    e->phy.vel.x = vel_mag * deg_cos(e->phy.yaw);
    e->phy.vel.y = vel_mag * deg_sin(e->phy.yaw);
}

static void enemy_asteroid_circle_spawn(game_state* game, screen_buffer* buffer, u8 n) {
    for (u8 i = 0; i < n; ++i) {
        game->enemies[i].type = ASTEROID_LARGE;
        const f32 pos_angle_from_origin = get_random_angle();
        const f32 dist_from_centre = buffer->height / 2.0f;
        const v2 pos =
            (v2){dist_from_centre * deg_cos(pos_angle_from_origin), dist_from_centre * deg_sin(pos_angle_from_origin)};

        enemy_asteroid_init(&game->enemies[i], ASTEROID_LARGE, pos);
    }
}

static void game_init(game_state* game, screen_buffer* buffer) {
    memset(game, 0, sizeof(game_state));

    game->mode = GAME_RUNNING;

    // init players
    game->players[0].alive = true;
    game->players[0].phy.pos = (v2){buffer->width / 2.0f, buffer->height / 2.0f};
    game->players[0].phy.yaw = 90.0f;
}

static bool player_all_dead(game_state* game) {
    bool all_dead = true;

    for (u8 p_i = 0; p_i < PLAYERS; ++p_i) {
        player_state* p = &game->players[p_i];
        if (p->alive || p->lives > 0) {
            all_dead = false;
            break;
        }
    }

    return all_dead;
}

static u16 enemy_count(game_state* game) {
    u16 count = 0;

    for (u16 i = 0; i < MAX_ENEMIES; ++i) {
        enemy_state* e = &game->enemies[i];
        if (e->type != DEAD) {
            ++count;
        }
    }

    return count;
}

static void render_debug_overlay(screen_buffer* buffer, game_state* game) {
    controller_state* p1_controller = &game->controllers[0];
    player_state* p1 = &game->players[0];

    wchar_t msg_buf[100] = {0};

    // render debug numbers
    for (u8 i = 0; i < buffer->width; i++) {
        wchar_t c = L'0' + (i % 10);
        printwc_xy(buffer, i, 0, c);
    }

    swprintf(msg_buf, sizeof(msg_buf), L"Total Enemies: %d", enemy_count(game));
    print_xy(buffer, 0, 4, msg_buf, wcslen(msg_buf));
    swprintf(msg_buf, sizeof(msg_buf), L"Yaw: %.2f", p1->phy.yaw);
    print_xy(buffer, 0, 3, msg_buf, wcslen(msg_buf));
    swprintf(msg_buf, sizeof(msg_buf), L"X: %.2f, Y: %.2f", p1->phy.pos.x, p1->phy.pos.y);
    print_xy(buffer, 0, 2, msg_buf, wcslen(msg_buf));
    swprintf(msg_buf, sizeof(msg_buf), L"VelX: %.2f, VelY: %.2f Mag: %.2f", p1->phy.vel.x, p1->phy.vel.y,
             v2_mag(p1->phy.vel));
    print_xy(buffer, 0, 1, msg_buf, wcslen(msg_buf));

    // visualise keyboard input
    buffer->data[buffer->size - 4] = p1_controller->shoot ? DARK_SHADE : LIGHT_SHADE;
    buffer->data[buffer->size - 3] = p1_controller->left ? DARK_SHADE : LIGHT_SHADE;
    buffer->data[buffer->size - 2] = p1_controller->up ? DARK_SHADE : LIGHT_SHADE;
    buffer->data[buffer->size - 1] = p1_controller->right ? DARK_SHADE : LIGHT_SHADE;
}

static void render_bullets(screen_buffer* buffer, game_state* game) {
    for (u8 p_i = 0; p_i < PLAYERS; ++p_i) {
        player_state* player = &game->players[p_i];

        for (u8 i = 0; i < MAX_BULLETS; ++i) {
            if (player->bullets[i].life_frames <= 0) {
                continue;
            }

            const usize x = (usize)player->bullets[i].phy.pos.x;
            const usize y = (buffer->height - 1) - (usize)player->bullets[i].phy.pos.y;
            const usize index = (y * buffer->width) + x;
            buffer->data[index] = U_BULLET;
        }
    }

    for (u8 e_i = 0; e_i < MAX_ENEMIES; ++e_i) {
        enemy_state* enemy = &game->enemies[e_i];
        if (enemy->type != SAUCER_LARGE && enemy->type != SAUCER_SMALL) {
            continue;
        }

        if (enemy->saucer_bullet.life_frames <= 0) {
            continue;
        }

        const usize x = (usize)enemy->saucer_bullet.phy.pos.x;
        const usize y = (buffer->height - 1) - (usize)enemy->saucer_bullet.phy.pos.y;
        const usize index = (y * buffer->width) + x;
        buffer->data[index] = U_BULLET;
    }
}

static void update_position(screen_buffer* buffer, physics* phy) {
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

static bool bullet_collision(v2 target_pos, v2 target_size, v2 bullet_pos) {
    return (target_pos.x <= bullet_pos.x && target_pos.x + target_size.x >= bullet_pos.x &&
            target_pos.y <= bullet_pos.y && target_pos.y + target_size.y >= bullet_pos.y);
}

static enemy_state* get_dead_enemy(game_state* game) {
    enemy_state* e = NULL;
    for (u8 i = 0; i < MAX_ENEMIES; ++i) {
        e = &(game->enemies[i]);
        if (e->type == DEAD) {
            break;
        }
    }

    assert(e != NULL);
    return e;
}

static bool player_collision(v2 player_pos, v2 player_size, v2 enemy_pos, v2 enemy_size) {
    f32 p_top = player_pos.y + player_size.y;
    f32 p_bottom = player_pos.y;
    f32 p_left = player_pos.x;
    f32 p_right = player_pos.x + player_size.x;

    f32 e_top = enemy_pos.y + enemy_size.y;
    f32 e_bottom = enemy_pos.y;
    f32 e_left = enemy_pos.x;
    f32 e_right = enemy_pos.x + enemy_size.x;

    if (p_right < e_left || e_right < p_left) {
        return false;
    }

    if (p_top < e_bottom || e_top < p_bottom) {
        return false;
    }

    return true;
}

static void enemy_kill(game_state* game, enemy_state* e) {
    switch (e->type) {
        case ASTEROID_SMALL: {
            e->type = DEAD;
            break;
        }
        case ASTEROID_MEDIUM: {
            // init first split asteroid
            enemy_asteroid_init(e, ASTEROID_SMALL, e->phy.pos);

            // init second split asteroid
            enemy_state* new_enemy = get_dead_enemy(game);
            enemy_asteroid_init(new_enemy, ASTEROID_SMALL, e->phy.pos);

            break;
        }
        case ASTEROID_LARGE: {
            // init first split asteroid
            enemy_asteroid_init(e, ASTEROID_MEDIUM, e->phy.pos);

            // init second split asteroid
            enemy_state* new_enemy = get_dead_enemy(game);
            enemy_asteroid_init(new_enemy, ASTEROID_MEDIUM, e->phy.pos);

            break;
        }

        case SAUCER_SMALL: {
            e->type = DEAD;
            break;
        }
        case SAUCER_LARGE: {
            e->type = DEAD;
            break;
        }
        case DEAD:
        default: {
            fprintf(stderr, "Error: unhandled case %d\n", (u32)e->type);
            exit(1);
        }
    }
}

static void enemy_spawn_random_large_asteroid(game_state* game, screen_buffer* buffer) {
    enemy_state* e = get_dead_enemy(game);

    const bool top_spawn = rand() % 2;
    u32 x;
    u32 y;

    if (top_spawn) {
        x = rand() % buffer->width;
        y = buffer->height - 1;
    } else {
        x = buffer->width - 1;
        y = rand() % buffer->height;
    }

    enemy_asteroid_init(e, ASTEROID_LARGE, (v2){x, y});
}

static void update_enemies(screen_buffer* buffer, game_state* game) {
    for (u8 i = 0; i < MAX_ENEMIES; ++i) {
        enemy_state* e = &game->enemies[i];
        if (e->type == DEAD) {
            continue;
        }

        // if saucer store last position
        if (e->type == SAUCER_SMALL || e->type == SAUCER_LARGE) {
            e->pos_last = e->phy.pos;
        }

        update_position(buffer, &e->phy);

        // check for collision with player
        for (u8 p_i = 0; p_i < PLAYERS; ++p_i) {
            player_state* p = &game->players[p_i];
            if (p->alive == false) {
                continue;
            }

            if (player_collision(p->phy.pos, PLAYER_SIZE, e->phy.pos, ENEMY_SIZE[e->type])) {
                player_kill(p);
                enemy_kill(game, e);
            }
        }

        // update saucer behaviour
        if (e->type == SAUCER_SMALL || e->type == SAUCER_LARGE) {
            // check if at edge of screen and position has wrapped
            const bool reached_end_going_left = (e->phy.vel.x < 0 && e->phy.pos.x > e->pos_last.x);
            const bool reached_end_going_right = (e->phy.vel.x >= 0 && e->phy.pos.x < e->pos_last.x);
            if (reached_end_going_left || reached_end_going_right) {
                e->type = DEAD;
            } else {
                if (e->direction_change_frames <= 0) {
                    e->direction_change_frames = 120;

                    const f32 vel_mag = ENEMY_MAX_VEL[e->type];
                    // pick a direction in a 90 degree cone centred on 0
                    e->phy.yaw = (f32)(rand() % 90) - 45.0f;
                    // adjust angle for direction
                    if (e->phy.vel.x < 0.0f) e->phy.yaw += 180.0f;
                    e->phy.yaw = degrees_clip(e->phy.yaw);

                    e->phy.vel.x = vel_mag * deg_cos(e->phy.yaw);
                    e->phy.vel.y = vel_mag * deg_sin(e->phy.yaw);
                }

                e->direction_change_frames -= 1;

                // shoot towards closest player in random spread
                if (e->saucer_bullet.life_frames <= 0) {
                    f32 min_mag = INFINITY;
                    v2 min_mag_vec = {0};
                    bool player_alive = false;
                    for (u8 p_i = 0; p_i < PLAYERS; ++p_i) {
                        player_state* player = &game->players[p_i];
                        if (player->alive == false) {
                            continue;
                        }

                        player_alive = true;
                        v2 enemy_to_player_vec =
                            (v2){player->phy.pos.x - e->phy.pos.x, player->phy.pos.y - e->phy.pos.y};

                        f32 mag = v2_mag(enemy_to_player_vec);
                        if (mag < min_mag) {
                            min_mag = mag;
                            min_mag_vec = enemy_to_player_vec;
                        }
                    }

                    if (player_alive) {
                        // spawn bullet
                        e->saucer_bullet.life_frames = BULLET_LIFE_FRAMES;
                        f32 bullet_yaw = deg_atan2(min_mag_vec.y, min_mag_vec.x / CHAR_SIZE_FACTOR);
                        bullet_yaw = degrees_clip(bullet_yaw);

                        // add spread to shot
                        static const f32 deg_spread = 30.0f;
                        f32 spread = fmodf((f32)rand(), deg_spread * 10.0f) / 10.0f;  // get rand float to 0.1 precision
                        spread = spread - deg_spread / 2.0f;                          // shift midpoint to 0.0
                        bullet_yaw += spread;

                        e->saucer_bullet.phy.pos = e->phy.pos;
                        e->saucer_bullet.phy.vel =
                            (v2){BULLET_SPEED * deg_cos(bullet_yaw), BULLET_SPEED * deg_sin(bullet_yaw)};
                    }
                } else {
                    e->saucer_bullet.life_frames -= 1;
                }

                // update bullet position
                update_position(buffer, &e->saucer_bullet.phy);

                // check saucer bullet for collisions
                for (u8 p_i = 0; p_i < PLAYERS; ++p_i) {
                    player_state* p = &game->players[p_i];
                    if (bullet_collision(p->phy.pos, PLAYER_SIZE, e->saucer_bullet.phy.pos)) {
                        e->saucer_bullet.life_frames = 0;
                        player_kill(p);
                    }
                }
            }
        }

        // check bullet collisions
        for (u8 j = 0; j < PLAYERS; ++j) {
            for (u8 k = 0; k < MAX_BULLETS; ++k) {
                bullet* b = &game->players[j].bullets[k];
                if (b->life_frames == 0) {
                    continue;
                }

                if (bullet_collision(e->phy.pos, ENEMY_SIZE[e->type], b->phy.pos)) {
                    // add score to player
                    player_state* p = &game->players[j];
                    p->score += ENEMY_SCORES[e->type];

                    // remove bullet
                    b->life_frames = 0;
                    game->enemies_shot += 1;

                    enemy_kill(game, e);

                    // current enemy has been collided with, don't continue iterating
                    j = PLAYERS;
                    k = MAX_BULLETS;
                }
            }
        }
    }

    // NOTE: the behaviour triggered by the largest number of enemies shot
    // will reset the counter to not rely on wrapping

    // spawn saucer if enough enemies have been shot
    if (game->enemies_shot == 15) {
        game->enemies_shot = 0;

        enemy_state* e = get_dead_enemy(game);
        e->type = SAUCER_LARGE;

        // spawn a small saucer every few saucers spawned
        game->enemies_saucers_spawned += 1;
        if (game->enemies_saucers_spawned == 3) {
            game->enemies_saucers_spawned = 0;
            e->type = SAUCER_SMALL;
        }

        enemy_saucer_init(buffer, e);
    } else if ((game->enemies_shot == 7 || game->enemies_shot == 14) && game->enemies_asteroids_left > 0) {
        // spawn a new large asteroid once a number of enemies has been shot
        game->enemies_asteroids_left -= 1;
        enemy_spawn_random_large_asteroid(game, buffer);
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

static void game_start_next_level(game_state* game, screen_buffer* buffer) {
    game->level += 1;
    game->level_delay_frames = 180;
    game->enemies_asteroids_left = game->enemies_asteroids_current_limit;
    game->enemies_asteroids_current_limit += 1;

    enemy_asteroid_circle_spawn(game, buffer, NEW_LEVEL_ASTEROID_COUNT);
}

static void render_gameover_screen(screen_buffer* buffer) {
    static const wchar_t* game_over_msg = L"GAME OVER";
    static const usize game_over_len = 9;
    print_xy(buffer, buffer->width / 2 - game_over_len / 2 + 1, buffer->height / 2 - 1, game_over_msg, game_over_len);
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

    // decrement level delay counter and check
    if (enemy_count(game) <= 0) {
        if (game->level_delay_frames == 0) {
            game_start_next_level(game, buffer);
        } else {
            game->level_delay_frames -= 1;
        }
    }

    update_player(player1, keyboard_controller_state, buffer);

    update_position(buffer, &player1->phy);
    update_bullets(buffer, player1->bullets);
    update_enemies(buffer, game);

    render_debug_overlay(buffer, game);
    render_ui(buffer, game);
    render_enemies(buffer, game->enemies);
    render_bullets(buffer, game);
    render_player(buffer, player1);

    if (player_all_dead(game)) {
        if (game->level_delay_frames == 0) {
            render_gameover_screen(buffer);
        } else {
            game->level_delay_frames -= 1;
        }
    }
}
