#include "asciiroids.h"
#include "constants.h"
#include "types.h"

static void update_player(player_state* player, controller_state* controller, screen_buffer* buffer) {
    static const f32 MAX_VEL_MAG = 20.0f;

    // check if player is currently respawning
    if (player->respawn_frames > 0) {
        player->respawn_frames -= 1;

        // TODO: respawn player according to starting spawn position
        // respawn player
        if (player->respawn_frames <= 0 && player->lives > 0) {
            player->alive = true;
            player->lives -= 1;
            player->shot_cooloff_frames = 0;
            player->phy.pos = (v2){buffer->width / 2, buffer->height / 2};
            player->phy.vel = (v2){0.0f, 0.0f};
            player->phy.yaw = 90.0f;
        }
    }

    if (player->alive == false) {
        return;
    }

    // handle boost
    // if boost reaches max velocity start biasing velocity in the player's current direction this will prevent the
    // player not being able to move in the desired direction when max velocity is reached.
    if (controller->up) {
        v2 vel_new = (v2){player->phy.vel.x + BOOST_ACCELERATION * cosf(to_radians(player->phy.yaw)) / FPS,
                          player->phy.vel.y + BOOST_ACCELERATION * sinf(to_radians(player->phy.yaw)) / FPS};

        const f32 vel_new_mag = v2_mag(vel_new);
        if (vel_new_mag <= MAX_VEL_MAG) {
            player->phy.vel = vel_new;
        } else {
            // remove velocity from where the player is currently moving
            player->phy.vel.x -= BOOST_ACCELERATION * (player->phy.vel.x / MAX_VEL_MAG) / FPS;
            player->phy.vel.y -= BOOST_ACCELERATION * (player->phy.vel.y / MAX_VEL_MAG) / FPS;
            const f32 vel_subbed_mag = v2_mag(player->phy.vel);

            // add velocity to where the player wants to go
            player->phy.vel.x += BOOST_ACCELERATION * cosf(to_radians(player->phy.yaw)) / FPS;
            player->phy.vel.y += BOOST_ACCELERATION * sinf(to_radians(player->phy.yaw)) / FPS;

            // cap magnitude of velocity to handle float precision errors
            while (v2_mag(player->phy.vel) > MAX_VEL_MAG) {
                player->phy.vel.x -= 0.01f;
                player->phy.vel.y -= 0.01f;
            }
        }
    }

    // handle turning
    static const f32 YAW_TICK = YAW_DEG_PER_SEC / FPS;
    if (controller->left && !controller->right) {
        player->phy.yaw += YAW_TICK;
        player->phy.yaw = degrees_clip(player->phy.yaw);
    } else if (controller->right && !controller->left) {
        player->phy.yaw -= YAW_TICK;
        player->phy.yaw = degrees_clip(player->phy.yaw);
    }

    // handle shooting
    if (controller->shoot) {
        if (player->shot_cooloff_frames == 0) {
            static const u16 shot_cooloff_frames = 5;
            player->shot_cooloff_frames = shot_cooloff_frames;

            // find a bullet slot that has expired
            for (u8 i = 0; i < MAX_BULLETS; ++i) {
                if (player->bullets[i].life_frames == 0) {
                    player->bullets[i].life_frames = BULLET_LIFE_FRAMES;
                    player->bullets[i].phy.pos = player->phy.pos;
                    player->bullets[i].phy.yaw = player->phy.yaw;
                    player->bullets[i].phy.vel = (v2){BULLET_SPEED * deg_cos(player->bullets[i].phy.yaw),
                                                      BULLET_SPEED * deg_sin(player->bullets[i].phy.yaw)};

                    break;
                }
            }
        } else {
            player->shot_cooloff_frames -= 1;
        }
    }
}

static void render_player(screen_buffer* buffer, player_state* player) {
    if (player->alive == false) {
        return;
    }
    if (player->respawn_frames > 0) {
        return;
    }

    const usize x = (usize)player->phy.pos.x;
    const usize y = (buffer->height - 1) - (usize)player->phy.pos.y;
    const usize index = (y * buffer->width) + x;

    assert(index >= 0);
    assert(index < buffer->size);

    static const f32 deg_per_segment = DEG_360 / (f32)TOTAL_DIRECTIONS;
    const f32 deg_half_seg = deg_per_segment / 2.0f;

    // offset yaw by half seg to make math easier, so there's no discontinuity before 0
    f32 yaw = player->phy.yaw + deg_half_seg;
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
