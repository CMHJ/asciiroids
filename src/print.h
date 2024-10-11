#ifndef ASCIIROIDS_PRINT_H
#define ASCIIROIDS_PRINT_H

#include "asciiroids.h"
#include "types.h"
#include "constants.h"

static void print_xy(screen_buffer* buffer, u32 x, u32 y, wchar_t* string, usize n);
static void printwc_xy(screen_buffer* buffer, u32 x, u32 y, wchar_t c);

#endif  // ASCIIROIDS_PRINT_H
