#ifndef ASCIIROIDS_GAME_H
#define ASCIIROIDS_GAME_H

#include "types.h"

const wchar_t light_shade = L'\u2591';
const wchar_t dark_shade = L'\u2593';

void run_game_loop(game_state* state, screen_buffer* buffer);

#endif  // ASCIIROIDS_GAME_H
