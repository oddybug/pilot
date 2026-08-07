#ifndef TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <uchar.h>

// Integers Unsigned

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

// Integers Signed

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

// Cahrs
typedef char c8;
typedef char16_t c16;
typedef char32_t c32;

// Floats

typedef float f32;
typedef double f64;

#ifdef __cplusplus
}
#endif

#endif // !TYPES_H
