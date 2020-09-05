#ifndef ady_TYPES_H
#define ady_TYPES_H

#include <stdint.h>

typedef uint8_t   u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float   f32;
typedef double  f64;

typedef u32     b32;

typedef unsigned char uchar;

#define false 0
#define true  1

#define internal static
#define local    static
#define global   static

/* A couple math things, maybe move this somewhere else */
#define PI 3.14159265359f
inline f32 DegToRad(f32 angle)
{
  return (angle * (PI / 180.0f));
}

#endif