#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#include <stdio.h>
#include "stdtypes.h"

#define CROP_NEWLINE(str, len) { \
    if ('\n' == str[len - 1])    \
        str[len - 1] = 0;        \
}

#define CROP_LAST_CHAR(str, len) { \
    str[len - 1] = 0;              \
}

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

#define max(a, b) ( \
{ \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; \
})

#define min(a, b) ( \
{ \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; \
})

#define isqr(val) ((val) * (val))

#define HEXDUMP_RECORD_LEN 0x10U

typedef enum {
    MOD_R = 0,
    MOD_W,
    MOD_A,
    MOD_RW,
    MOD_RA,
    MOD_RW_C,
    MOD_NO
} file_mode_t;

void str_reverse(char *str, int len);
int str_compare(const char *s1, const char *s2);

int get_word(const char *str, usize *pos, char *word, usize size);
int get_line(const char *filepath, usize *foffset, char *buff, usize size);

stdret_t hexdump(const char *filepath, const char *buff, usize size, const char *append);
stdret_t read_file(const char *filepath, char *buff, usize size, file_mode_t fmod);
stdret_t write_file(const char *filepath, const char *buff, usize size, file_mode_t fmod);

s64 merge_bytes(const u8 *bytes, const u8 bits);

#endif /* __UTILS_H__ */