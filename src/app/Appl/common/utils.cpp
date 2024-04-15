/**********************************************************************************************************************
* utils.cpp                                                                                                           *
***********************************************************************************************************************
* DESCRIPTION :                                                                                                       *
*       Utility functions used in hab project.                                                                        *
*                                                                                                                     *
* PUBLIC FUNCTIONS :                                                                                                  *
*       stdret_t            iiobuff_log2file(char *ubuff, const habdev_t *habdev)                                     *
*                                                                                                                     *
* AUTHOR :                                                                                                            *
*       Yahor Yauseyenka    email: yahoryauseyenka@gmail.com                                                          *
*                                                                                                                     *
* VERSION                                                                                                             *
*       0.0.1               last modification: 14-04-2024                                                             *
*                                                                                                                     *
* LICENSE                                                                                                             *
*       GPL                                                                                                           *
*                                                                                                                     *
***********************************************************************************************************************/


/**********************************************************************************************************************
 *  INCLUDES
 *********************************************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "utils.h"

#include <errno.h>
/**********************************************************************************************************************
 *  PREPROCESSOR DEFINITIONS
 *********************************************************************************************************************/
#define MAX_SUBSTR_SIZE 1024

#define CROP_NEWLINE(str, len) { \
    if ('\n' == str[len - 1])    \
        str[len - 1] = 0;        \
}


/**********************************************************************************************************************
 * GLOBAL VARIABLES DECLARATION
 *********************************************************************************************************************/
extern int errno;

 /**********************************************************************************************************************
 * LOCAL FUNCTION DECLARATION
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * LOCAL FUNCTION DEFINITION
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * GLOBAL FUNCTION DEFINITION
 *********************************************************************************************************************/
void str_reverse(char *str, int len) {
    int begin = 0;
    int end = len - 1;
    char tmp;
    
    while (end > begin) {
        tmp = str[begin];

        str[begin++] = str[end];
        str[end--] = tmp;
    }
}

void to_char(int num, char *str) {
    int len = 0;

    if (num == 0)
        str[0] = num + '0';
    
    while(num) {
        str[len++] = (num % 10) + '0';
        num /= 10;
    }

    str_reverse(str, len);
}

int get_word(const char *str, usize *pos, char *word, usize size) {
    int idx = 0;

    if (str[*pos] == 0)
        return -1;

    if (word)
        memset(word, 0, size);

    while(str[*pos] == ' ')
        (*pos)++;

    while(str[*pos] != ' ' && str[*pos] != 0) {
        word[idx++] = str[(*pos)++];
    }
    
    return 0;
}

int get_line(const char *filepath, usize *foffset, char *buff, usize size) {
    int ret = 0;
    char ch;
    usize i = 0;
    FILE *filp = NULL;

    memset(buff, 0, size);

    filp = fopen(filepath, "r");
    if (NULL == filp)
        return -2;
    fseek(filp, *foffset, SEEK_SET);

    for ( ; ch != '\n' && !feof(filp); i++) {
        ch = fgetc(filp);
        (*foffset)++;

        if ('\r' == ch) {
            buff[i] = 0;
        } else if('\n' == ch) {
            buff[i] = 0;
            ret = 0;
        } else if(!feof(filp)) {
            buff[i] = ch;
        } else {
            ret = -1;
        }
    }
    fclose(filp);
    return ret;
}

stdret_t read_file(const char *filepath, char *buff, usize size) {
    stdret_t ret = STD_NOT_OK;
    FILE *filp = NULL;

    filp = fopen(filepath, "r");
    if (NULL == filp) {
        fprintf(stderr, "ERROR: Could not open the file - %s\n", filepath);
        printf("errno %d\n", errno);
    } else {
        fread(buff, sizeof(char), size, filp);
        ret = STD_OK;
    }
    fclose(filp);

    return ret;
}

stdret_t write_file(const char *filepath, const char *buff, usize size) {
    stdret_t ret = STD_NOT_OK;
    int bytes_written = 0;
    FILE *filp = NULL;

    filp = fopen(filepath, "w");
    if (NULL == filp) {
        fprintf(stderr, "ERROR: Could not open the file for writing - %s\n", filepath);
    } else {
        bytes_written = fwrite(buff, sizeof(char), size, filp);
        if (bytes_written == size)
            ret = STD_OK;
    }
    fclose(filp);

    return ret;
}

stdret_t create_path(char *base, usize n, ...) {
    stdret_t ret = STD_NOT_OK;
    char *subpath = NULL;
    int i = 0;

    va_list list;
    va_start(list, n);

    memset(base, 0, strlen(base));

    for (i = 0; i < n; i++) {
        subpath = va_arg(list, char*);

        strcat(base, subpath);
    }

    va_end(list);

    CROP_NEWLINE(base, strlen(base));
    
    ret = STD_OK;
    return ret;
}
