#ifndef ASCIIROIDS_GAME_H
#define ASCIIROIDS_GAME_H

#include "types.h"

// TODO(CMHJ): replace this with some randomly generated starscape?
const wchar_t no_shade = L' ';
const wchar_t light_shade = L'\u2591';
const wchar_t dark_shade = L'\u2593';

const wchar_t U_BULLET = L'\u00B7';

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

typedef enum game_mode { GAME_NEW, GAME_RUNNING, GAME_QUIT } game_mode;

#define PLAYERS 1
typedef struct game_state {
    game_mode mode;
    i8 controller_fds[PLAYERS];
    controller_state controllers[PLAYERS];
    player_state players[PLAYERS];
} game_state;

// define run_game_loop function type and default stub function
#define RUN_GAME_LOOP(name) void name(game_state* state, screen_buffer* buffer)
typedef RUN_GAME_LOOP(run_game_loop_function_type);
RUN_GAME_LOOP(run_game_loop_stub) {
    (void)state;
    (void)buffer;
}

#endif  // ASCIIROIDS_GAME_H
