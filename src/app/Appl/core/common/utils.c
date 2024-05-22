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

/**********************************************************************************************************************
 * GLOBAL VARIABLES DECLARATION
 *********************************************************************************************************************/
extern int errno;
static const char *file_modes[MOD_NO] = {
    "r", "w", "a", "r+", "a+", "w+"
};

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

int str_compare(const char *s1, const char *s2) {
    while (*s1 == *s2++)
        if (*s1++ == '\0') {
            return (0);
        }

    return (*(const unsigned char *)s1 - *(const unsigned char *)(s2 - 1));
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
    char ch = '0';
    FILE *filp = NULL;

    memset(buff, 0, size);

    filp = fopen(filepath, "r");
    if (NULL == filp) {
        fprintf(stderr, "ERROR: Error opening the file. File: %s\n", filepath);
        return -2;
    }
    fseek(filp, *foffset, SEEK_SET);

    for (usize i = 0; ch != '\n' && !feof(filp); i++) {
        ch = fgetc(filp);
        (*foffset)++;

        if ('\r' == ch) {
            buff[i] = 0;
        } else if('\n' == ch) {
            buff[i] = '\n';
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

stdret_t hexdump(const char *filepath, const char *buff, usize size, const char *append) {
    stdret_t ret = STD_NOT_OK;
    u8 byte = 0;
    FILE *filp = NULL;

    filp = fopen(filepath, file_modes[MOD_A]);
    if (NULL == filp) {
        fprintf(stderr, "ERROR: Error opening the file: %s\n", filepath);
        return ret;
    }

    for (usize i = 0; i < size; i++) {
        for (usize j = 0; j < HEXDUMP_RECORD_LEN; j++) {
            byte = buff[j + i * HEXDUMP_RECORD_LEN];
    
            fprintf(filp, "%02x", byte);
            if (j % 2 != 0)
                fprintf(filp, " ");
        }
        if (NULL != append)
            fprintf(filp, "| %s\n", append);
        else
            fprintf(filp, "\n");
    }
    fclose(filp);

    ret = STD_OK;
    return ret;
}

stdret_t read_file(const char *filepath, char *buff, usize size, file_mode_t fmod) {
    stdret_t ret = STD_NOT_OK;
    FILE *filp = NULL;

    memset(buff, 0, strlen(buff));

    filp = fopen(filepath, file_modes[fmod]);
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

/* OPTIONAL: Add a conditinal check whether the mode allows to write or not. */
stdret_t write_file(const char *filepath, const char *buff, usize size, file_mode_t fmod) {
    stdret_t ret = STD_NOT_OK;
    int bytes_written = 0;
    FILE *filp = NULL;

    filp = fopen(filepath, file_modes[fmod]);
    if (NULL == filp) {
        fprintf(stderr, "ERROR: Could not open the file for writing - %s\n", filepath);
        return STD_NOT_OK;
    } else {
        bytes_written = fwrite(buff, sizeof(char), size, filp);
        if (bytes_written == size)
            ret = STD_OK;
    }
    fclose(filp);

    return ret;
}

s64 merge_bytes(const u8 *bytes, const u8 bits) {
    s64 ret = 0;

    for (int i = 0; i < (bits / 8); i++)
        ret |= (s64)bytes[i] << (i * 8);
    
    /* Handle TS differently? */
    if (ret & (1 << (bits - 1)))
        ret = -((1 << bits) - ret);
    
    return ret;
}
