#ifndef ASCIIROIDS_TYPES_H
#define ASCIIROIDS_TYPES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef size_t usize;
typedef ssize_t isize;

#define SCREEN_BUFFER_WIDTH 80
#define SCREEN_BUFFER_HEIGHT 24
typedef struct screen_buffer {
    const usize width;
    const usize height;
    const usize size;
    wchar_t data[SCREEN_BUFFER_WIDTH * SCREEN_BUFFER_HEIGHT];
} screen_buffer;

typedef struct v2 {
    f32 x;
    f32 y;
} v2;

typedef struct controller_state {
    bool up;
    bool left;
    bool right;
    bool shoot;
    bool quit;
} controller_state;

typedef struct player_state {
    v2 pos;
    f32 yaw;
    v2 vel;
} player_state;

#endif  // ASCIIROIDS_TYPES_H
