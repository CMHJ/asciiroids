#include "asciiroids.h"
#include "constants.h"
#include "types.h"

static void update_player_input(player_state* player, controller_state* controller) {
    static const f32 MAX_VEL_MAG = 20.0f;

    // TODO(CMHJ): refactor boost code
    // handle boost
    if (controller->up) {
        v2 vel_new = (v2){player->phy.vel.x + BOOST_ACCELERATION * cosf(to_radians(player->phy.yaw)) / FPS,
                          player->phy.vel.y + BOOST_ACCELERATION * sinf(to_radians(player->phy.yaw)) / FPS};

        const f32 vel_new_mag = v2_mag(vel_new);
        if (vel_new_mag <= MAX_VEL_MAG) {
            player->phy.vel = vel_new;
        } else {
            player->phy.vel.x -= BOOST_ACCELERATION * (player->phy.vel.x / MAX_VEL_MAG) / FPS;
            player->phy.vel.y -= BOOST_ACCELERATION * (player->phy.vel.y / MAX_VEL_MAG) / FPS;
            const f32 vel_subbed_mag = v2_mag(player->phy.vel);

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

        // clip yaw to [0, 360.0)
        if (player->phy.yaw >= DEG_360) {
            player->phy.yaw -= DEG_360;
        }
    } else if (controller->right && !controller->left) {
        player->phy.yaw -= YAW_TICK;

        if (player->phy.yaw < 0.0f) {
            player->phy.yaw += DEG_360;
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
                    player->bullets[i].phy.pos = player->phy.pos;
                    player->bullets[i].phy.yaw = player->phy.yaw;

                    static const f32 bullet_speed = 20.0f;
                    player->bullets[i].phy.vel = (v2){bullet_speed * cosf(to_radians(player->bullets[i].phy.yaw)),
                                                      bullet_speed * sinf(to_radians(player->bullets[i].phy.yaw))};

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

    player->phy.pos.x += (player->phy.vel.x * CHAR_SIZE_FACTOR) / FPS;
    if (player->phy.pos.x >= buffer->width) {
        player->phy.pos.x -= buffer->width;

    } else if (player->phy.pos.x < 0.0f) {
        player->phy.pos.x += buffer->width;
    }

    player->phy.pos.y += player->phy.vel.y / FPS;
    if (player->phy.pos.y >= buffer->height)
        player->phy.pos.y -= buffer->height;
    else if (player->phy.pos.y < 0.0f)
        player->phy.pos.y += buffer->height;
}

static void update_player_bullets(screen_buffer* buffer, player_state* player) {
    // account for there being a difference in the height and width of characters.
    // because chars are taller than they are wide, moving north/south is much faster than east/west.
    // this factor accounts for that to make the speed seem smooth
    static const f32 CHAR_SIZE_FACTOR = 2.0f;

    for (u8 i = 0; i < MAX_BULLETS; ++i) {
        bullet* b = &(player->bullets[i]);
        if (b->life_frames == 0) {
            continue;
        }

        // tick bullet life down each frame
        b->life_frames -= 1;

        b->phy.pos.x += (b->phy.vel.x * CHAR_SIZE_FACTOR) / FPS;
        if (b->phy.pos.x >= buffer->width) {
            b->phy.pos.x -= buffer->width;

        } else if (b->phy.pos.x < 0.0f) {
            b->phy.pos.x += buffer->width;
        }

        b->phy.pos.y += b->phy.vel.y / FPS;
        if (b->phy.pos.y >= buffer->height)
            b->phy.pos.y -= buffer->height;
        else if (b->phy.pos.y < 0.0f)
            b->phy.pos.y += buffer->height;
    }
}

static void render_player_ship(screen_buffer* buffer, player_state* player) {
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
