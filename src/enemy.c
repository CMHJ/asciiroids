#include <stdio.h>

#include "asciiroids.h"
#include "constants.h"
#include "types.h"

static void render_enemy_asteroid_small(screen_buffer* buffer, v2 pos) {
    // check if in top or bottom half of the terminal char
    if (fmodf(pos.y, 1.0f) < 0.5f) {
        printwc_xy(buffer, pos.x, pos.y, BLOCK_LOWER_HALF);
    } else {
        printwc_xy(buffer, pos.x, pos.y, BLOCK_UPPER_HALF);
    }
}

static void render_enemy_asteroid_medium(screen_buffer* buffer, v2 pos) {
    static wchar_t layer[2] = {BLOCK_FULL, BLOCK_FULL};
    print_xy(buffer, (usize)pos.x, (usize)pos.y, layer, 2);
}

static void render_enemy_asteroid_large(screen_buffer* buffer, v2 pos) {
    static wchar_t layer[4] = {BLOCK_FULL, BLOCK_FULL, BLOCK_FULL, BLOCK_FULL};
    print_xy(buffer, (usize)pos.x, (usize)pos.y, layer, 4);
    print_xy(buffer, (usize)pos.x, (usize)pos.y + 1, layer, 4);
}

static void render_enemy_saucer_small(screen_buffer* buffer, v2 pos) {
    static wchar_t layer[2] = {SAUCER_SMALL_LEFT, SAUCER_SMALL_RIGHT};
    print_xy(buffer, (usize)pos.x, (usize)pos.y, layer, 2);
}

static void render_enemy_saucer_large(screen_buffer* buffer, v2 pos) {
    static wchar_t layer[4] = {SAUCER_LARGE_LEFT, SAUCER_LARGE_LEFT_MID, SAUCER_LARGE_RIGHT_MID, SAUCER_LARGE_RIGHT};
    print_xy(buffer, (usize)pos.x, (usize)pos.y, layer, 4);
}

static void render_enemies(screen_buffer* buffer, enemy_state* enemies) {
    for (u8 i = 0; i < MAX_ENEMIES; ++i) {
        switch (enemies[i].type) {
            case DEAD: {
                // do nothing
                break;
            }
            case ASTEROID_SMALL: {
                render_enemy_asteroid_small(buffer, enemies[i].phy.pos);
                break;
            }
            case ASTEROID_MEDIUM: {
                render_enemy_asteroid_medium(buffer, enemies[i].phy.pos);
                break;
            }
            case ASTEROID_LARGE: {
                render_enemy_asteroid_large(buffer, enemies[i].phy.pos);
                break;
            }
            case SAUCER_SMALL: {
                render_enemy_saucer_small(buffer, enemies[i].phy.pos);
                break;
            }
            case SAUCER_LARGE: {
                render_enemy_saucer_large(buffer, enemies[i].phy.pos);
                break;
            }
            default: {
                fprintf(stderr, "Not implemented rendering for type: %d\n", enemies[i].type);
                exit(1);
            }
        }
    }
}
