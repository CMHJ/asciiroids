#include "utility.h"

#include <stdlib.h>

static inline f32 to_radians(const f32 degrees) {
    return degrees * (PI / 180.0f);
}

static inline f32 to_degrees(const f32 radians) {
    return radians * (180.0f / PI);
}

static inline f32 get_random_angle(void) {
    // NOTE: this will only give integer values
    return rand() % 360;
}

static inline f32 v2_mag(const v2 v) {
    return sqrtf((v.x * v.x) + (v.y * v.y));
}

static void buffer_clear(screen_buffer* buffer) {
    for (usize i = 0; i < buffer->width * buffer->height; ++i) {
        buffer->data[i] = NO_SHADE;
    }
}

static f32 degrees_clip(f32 angle) {
    while (angle >= DEG_360) {
        angle -= DEG_360;
    }
    while (angle < 0.0f) {
        angle += DEG_360;
    }

    return angle;
}

static f32 deg_sin(f32 x) {
    return sinf(to_radians(x));
}

static f32 deg_cos(f32 x) {
    return cosf(to_radians(x));
}

static f32 deg_atan2(f32 y, f32 x) {
    f32 angle = atan2f(y, x);
    angle = to_degrees(angle);
    return angle;
}
