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

typedef struct v2 {
    f32 x;
    f32 y;
} v2;

#endif  // ASCIIROIDS_TYPES_H
