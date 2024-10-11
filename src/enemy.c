#include <stdio.h>

#include "asciiroids.h"
#include "constants.h"
#include "types.h"

static void update_enemies(screen_buffer* buffer, enemy_state* enemies) {
    static const f32 CHAR_SIZE_FACTOR = 2.0f;
    // TODO(CMHJ): refactor all pos update functions into single function that updates physics

    for (u8 i = 0; i < MAX_ENEMIES; ++i) {
        enemy_state* e = &(enemies[i]);
        if (e->type == DEAD) {
            continue;
        }

        e->phy.pos.x += (e->phy.vel.x * CHAR_SIZE_FACTOR) / FPS;
        if (e->phy.pos.x >= buffer->width) {
            e->phy.pos.x -= buffer->width;

        } else if (e->phy.pos.x < 0.0f) {
            e->phy.pos.x += buffer->width;
        }

        e->phy.pos.y += e->phy.vel.y / FPS;
        if (e->phy.pos.y >= buffer->height)
            e->phy.pos.y -= buffer->height;
        else if (e->phy.pos.y < 0.0f)
            e->phy.pos.y += buffer->height;
    }
}

static void render_enemy_asteroid_large(screen_buffer* buffer, v2 pos) {
    static wchar_t layer[4] = {BLOCK_FULL, BLOCK_FULL, BLOCK_FULL, BLOCK_FULL};
    print_xy(buffer, (usize)pos.x, (usize)pos.y, layer, 4);
    print_xy(buffer, (usize)pos.x, (usize)pos.y + 1, layer, 4);
}

static void render_enemies(screen_buffer* buffer, enemy_state* enemies) {
    for (u8 i = 0; i < MAX_ENEMIES; ++i) {
        switch (enemies[i].type) {
            case DEAD: {
                // do nothing
                break;
            }
            case ASTEROID_LARGE: {
                render_enemy_asteroid_large(buffer, enemies[i].phy.pos);
                break;
            }
            default: {
                fprintf(stderr, "Not implemented rendering for type: %d\n", enemies[i].type);
                exit(1);
            }
        }
    }
}
