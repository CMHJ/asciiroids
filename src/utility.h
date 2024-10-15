#ifndef ASCIIROIDS_UTILITY_H
#define ASCIIROIDS_UTILITY_H

#include "asciiroids.h"
#include "types.h"
#include "constants.h"

static inline f32 to_radians(const f32 degrees);
static inline f32 v2_mag(const v2 v);
static void buffer_clear(screen_buffer* buffer);
static f32 degrees_clip(f32 angle);

#endif  // ASCIIROIDS_UTILITY_H
