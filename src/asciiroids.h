#ifndef ASCIIROIDS_GAME_H
#define ASCIIROIDS_GAME_H

#include "types.h"

// TODO(CMHJ): replace this with some randomly generated starscape?
const wchar_t NO_SHADE = L' ';
const wchar_t LIGHT_SHADE = L'\u2591';
const wchar_t DARK_SHADE = L'\u2593';

const wchar_t BLOCK_FULL = L'\u2588';
const wchar_t BLOCK_UPPER_HALF = L'\u2580';
const wchar_t BLOCK_LOWER_HALF = L'\u2584';

const wchar_t SAUCER_LARGE_LEFT = L'\U0001FB46';
const wchar_t SAUCER_LARGE_LEFT_MID = L'\U0001FB41';
const wchar_t SAUCER_LARGE_RIGHT_MID = L'\U0001FB4C';
const wchar_t SAUCER_LARGE_RIGHT = L'\U0001FB51';

const wchar_t SAUCER_SMALL_LEFT = L'\U0001FB46';
const wchar_t SAUCER_SMALL_RIGHT = L'\U0001FB51';

const wchar_t U_BULLET = L'\u00B7';

#define NUMBERS_NUM 10
const wchar_t NUMBERS[NUMBERS_NUM] = {
    L'\U0001FBF0', L'\U0001FBF1', L'\U0001FBF2', L'\U0001FBF3', L'\U0001FBF4',
    L'\U0001FBF5', L'\U0001FBF6', L'\U0001FBF7', L'\U0001FBF8', L'\U0001FBF9',
};

// describe segment which is the direction that the ship is pointing
typedef enum ship_direction {
    EAST = 0,
    NORTH_EAST,
    NORTH,
    NORTH_WEST,
    WEST,
    SOUTH_WEST,
    SOUTH,
    SOUTH_EAST,
    TOTAL_DIRECTIONS
} ship_direction;

const wchar_t U_SHIP_N = L'\u25B2';
const wchar_t U_SHIP_E = L'\u25BA';
const wchar_t U_SHIP_S = L'\u25BC';
const wchar_t U_SHIP_W = L'\u25C4';
const wchar_t U_SHIP_SE = L'\u25E2';
const wchar_t U_SHIP_SW = L'\u25E3';
const wchar_t U_SHIP_NW = L'\u25E4';
const wchar_t U_SHIP_NE = L'\u25E5';

// starts east at 0 degrees and goes counter clockwise
const wchar_t SHIP_CHARS[TOTAL_DIRECTIONS] = {U_SHIP_E, U_SHIP_NE, U_SHIP_N, U_SHIP_NW,
                                              U_SHIP_W, U_SHIP_SW, U_SHIP_S, U_SHIP_SE};

#define FPS 60

#define SCREEN_BUFFER_WIDTH 80
#define SCREEN_BUFFER_HEIGHT 24
typedef struct screen_buffer {
    const usize width;
    const usize height;
    const usize size;
    wchar_t data[SCREEN_BUFFER_WIDTH * SCREEN_BUFFER_HEIGHT];
} screen_buffer;

typedef struct controller_state {
    bool up;
    bool left;
    bool right;
    bool shoot;
    bool quit;
} controller_state;

typedef struct physics {
    v2 pos;
    f32 yaw;
    v2 vel;
} physics;

typedef struct bullet {
    physics phy;
    u16 life_frames;
} bullet;

#define MAX_BULLETS 4
typedef struct player_state {
    physics phy;
    bullet bullets[MAX_BULLETS];
    u16 shot_cooloff_frames;
} player_state;

typedef enum enemy_type {
    DEAD,
    ASTEROID_SMALL,
    ASTEROID_MEDIUM,
    ASTEROID_LARGE,
    SAUCER_SMALL,
    SAUCER_LARGE,
} enemy_type;

typedef struct enemy_state {
    enemy_type type;
    physics phy;
} enemy_state;

typedef enum game_mode { GAME_NEW, GAME_RUNNING, GAME_QUIT } game_mode;

#define PLAYERS 1
#define MAX_ENEMIES 20
typedef struct game_state {
    game_mode mode;
    i8 controller_fds[PLAYERS];
    controller_state controllers[PLAYERS];
    player_state players[PLAYERS];
    enemy_state enemies[MAX_ENEMIES];
} game_state;

// define run_game_loop function type and default stub function
#define RUN_GAME_LOOP(name) void name(game_state* state, screen_buffer* buffer)
typedef RUN_GAME_LOOP(run_game_loop_function_type);
RUN_GAME_LOOP(run_game_loop_stub) {
    (void)state;
    (void)buffer;
}

#endif  // ASCIIROIDS_GAME_H
