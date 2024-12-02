#ifndef ASCIIROIDS_GAME_H
#define ASCIIROIDS_GAME_H

#include "types.h"

// TODO(CMHJ): replace this with some randomly generated starscape?
static const wchar_t NO_SHADE = L' ';
static const wchar_t LIGHT_SHADE = L'\u2591';
static const wchar_t DARK_SHADE = L'\u2593';

static const wchar_t BLOCK_FULL = L'\u2588';
static const wchar_t BLOCK_UPPER_HALF = L'\u2580';
static const wchar_t BLOCK_LOWER_HALF = L'\u2584';

static const wchar_t SAUCER_LARGE_LEFT = L'\U0001FB46';
static const wchar_t SAUCER_LARGE_LEFT_MID = L'\U0001FB41';
static const wchar_t SAUCER_LARGE_RIGHT_MID = L'\U0001FB4C';
static const wchar_t SAUCER_LARGE_RIGHT = L'\U0001FB51';

static const wchar_t SAUCER_SMALL_LEFT = L'\U0001FB46';
static const wchar_t SAUCER_SMALL_RIGHT = L'\U0001FB51';

static const wchar_t U_BULLET = L'\u00B7';

// TODO: remove this
#define NUMBERS_NUM 10
static const wchar_t NUMBERS[NUMBERS_NUM] = {
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

static const wchar_t U_SHIP_N = L'\u25B2';
static const wchar_t U_SHIP_E = L'\u25BA';
static const wchar_t U_SHIP_S = L'\u25BC';
static const wchar_t U_SHIP_W = L'\u25C4';
static const wchar_t U_SHIP_SE = L'\u25E2';
static const wchar_t U_SHIP_SW = L'\u25E3';
static const wchar_t U_SHIP_NW = L'\u25E4';
static const wchar_t U_SHIP_NE = L'\u25E5';

// starts east at 0 degrees and goes counter clockwise
static const wchar_t SHIP_CHARS[TOTAL_DIRECTIONS] = {U_SHIP_E, U_SHIP_NE, U_SHIP_N, U_SHIP_NW,
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
    bool alive;
    u8 lives;
    u16 shot_cooloff_frames;
    u16 respawn_frames;
    u32 score;
} player_state;

static const v2 PLAYER_SIZE = {1.0f, 1.0f};

typedef enum enemy_type {
    DEAD,
    ASTEROID_SMALL,
    ASTEROID_MEDIUM,
    ASTEROID_LARGE,
    SAUCER_SMALL,
    SAUCER_LARGE,
    TOTAL_ENEMY_TYPES
} enemy_type;

typedef struct enemy_state {
    enemy_type type;
    physics phy;

    // saucer specific data
    v2 pos_last;
    u16 direction_change_frames;
    bullet saucer_bullet;
} enemy_state;

static const v2 ENEMY_SIZE[TOTAL_ENEMY_TYPES] = {
    {0.0f, 0.0f}, {1.0f, 0.5f}, {2.0f, 1.0f}, {4.0f, 2.0f}, {2.0f, 1.0f}, {4.0f, 1.0f},
};

static const f32 ENEMY_MAX_VEL[TOTAL_ENEMY_TYPES] = {0.0f, 9.0f, 6.0f, 3.0f, 6.0f, 4.0f};

static const u8 ENEMY_SCORES[TOTAL_ENEMY_TYPES] = {0, 10, 10, 10, 50, 100};

typedef enum game_mode { GAME_NEW, GAME_MAIN_MENU, GAME_RUNNING, GAME_QUIT } game_mode;

// TODO: change this to max players and allow multiplayer
#define PLAYERS 1
#define MAX_ENEMIES 100
#define NEW_LEVEL_ASTEROID_COUNT 4
typedef struct game_state {
    game_mode mode;
    u8 menu_selection;
    u8 level;
    controller_state controllers[PLAYERS];
    player_state players[PLAYERS];
    enemy_state enemies[MAX_ENEMIES];
    i8 enemies_shot;  // TODO: change these to unsigned?
    i8 enemies_saucers_spawned;
    u8 enemies_asteroids_current_limit;
    u8 enemies_asteroids_left;
    u16 level_delay_frames;
} game_state;

// define run_game_loop function type and default stub function
#define RUN_GAME_LOOP(name) void name(game_state* game, screen_buffer* buffer)
typedef RUN_GAME_LOOP(run_game_loop_function_type);
RUN_GAME_LOOP(run_game_loop_stub) {
    (void)game;
    (void)buffer;
}

#endif  // ASCIIROIDS_GAME_H
