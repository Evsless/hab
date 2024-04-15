#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#include <stdio.h>
#include "stdtypes.h"

void to_char(int num, char *str);
void str_reverse(char *str, int len);

int get_word(const char *str, usize *pos, char *word, usize size);
int get_line(const char *filepath, usize *foffset, char *buff, usize size);

stdret_t read_file(const char *filepath, char *buff, usize size);
stdret_t write_file(const char *filepath, const char *buff, usize size);

stdret_t create_path(char *base, usize n, ...);


#endif /* __UTILS_H__ */