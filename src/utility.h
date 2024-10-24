#ifndef ASCIIROIDS_UTILITY_H
#define ASCIIROIDS_UTILITY_H

#include "asciiroids.h"
#include "types.h"
#include "constants.h"

static inline f32 to_radians(const f32 degrees);
static inline f32 to_degrees(const f32 radians);
static inline f32 get_random_angle(void);
static inline f32 v2_mag(const v2 v);
static void buffer_clear(screen_buffer* buffer);
static f32 degrees_clip(f32 angle);
static f32 deg_sin(f32 x);
static f32 deg_cos(f32 x);
static f32 deg_atan2(f32 y, f32 x);

#endif  // ASCIIROIDS_UTILITY_H
