#ifndef ASCIIROIDS_GAME_H
#define ASCIIROIDS_GAME_H

#include "types.h"

const wchar_t light_shade = L'\u2591';
const wchar_t dark_shade = L'\u2593';

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
