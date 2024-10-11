#include "utility.h"

static inline f32 to_radians(const f32 degrees) {
    return degrees * (PI / 180.0f);
}

static inline f32 v2_mag(const v2 v) {
    return sqrtf((v.x * v.x) + (v.y * v.y));
}

static void buffer_clear(screen_buffer* buffer) {
    for (usize i = 0; i < buffer->width * buffer->height; ++i) {
        buffer->data[i] = NO_SHADE;
    }
}
