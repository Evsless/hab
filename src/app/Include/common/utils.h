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


void to_char(int num, char *str);
void str_reverse(char *str, int len);
int str_compare(const char *s1, const char *s2);

int get_word(const char *str, usize *pos, char *word, usize size);
int get_line(const char *filepath, usize *foffset, char *buff, usize size);

stdret_t read_file(const char *filepath, char *buff, usize size);
stdret_t write_file(const char *filepath, const char *buff, usize size);

stdret_t create_path(char *base, usize n, ...);


#endif /* __UTILS_H__ */