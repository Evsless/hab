#ifndef __STDTYPES_H__
#define __STDTYPES_H__

#include <stdint.h>

typedef size_t      usize;

typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

typedef int8_t      s8;
typedef int16_t     s16;
typedef int32_t     s32;
typedef int64_t     s64;

#define MILLI 1000LU
#define MICRO 1000000LU
#define NANO  1000000000LU

typedef enum {
    STD_OK,
    STD_NOT_OK,
} stdret_t;


#endif /* __STDTYPES_H__ */