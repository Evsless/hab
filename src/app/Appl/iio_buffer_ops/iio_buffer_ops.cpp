/**********************************************************************************************************************
* iio_buffer_ops.cpp                                                                                                  *
***********************************************************************************************************************
* DESCRIPTION :                                                                                                       *
*       Industrial I/O control routines. Routines are used when handling the data stored inside                       *
*       iio triggered buffer.                                                                                         *
*                                                                                                                     *
* PUBLIC FUNCTIONS :                                                                                                  *
*       stdret_t            iiobuff_log2file(char *ubuff, const habdev_t *habdev)                                     *
*                                                                                                                     *
* AUTHOR :                                                                                                            *
*       Yahor Yauseyenka    email: yahoryauseyenka@gmail.com                                                          *
*                                                                                                                     *
* VERSION                                                                                                             *
*       0.0.1               last modification: 10-04-2024                                                             *
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
#include <unistd.h>

#include "iio_buffer_ops.h"

/**********************************************************************************************************************
 *  MACRO
 *********************************************************************************************************************/
#define HEXDUMP_RECORD_LEN 0x10
#define BUFF_LEN_SUBPATH   "buffer/data_available"

/**********************************************************************************************************************
 * LOCAL FUNCTION DECLARATION
 *********************************************************************************************************************/
static void flip_nibbles(char *buffer, usize size);
static stdret_t get_buff_size(const habdev_t *habdev);
static stdret_t hexdump_to_file(void *buffer, const int size, const habdev_t *habdev);

/**********************************************************************************************************************
 * LOCAL FUNCTION DEFINITION
 *********************************************************************************************************************/
static void flip_nibbles(char *buffer, usize size) {
    char tmp;
    for (usize i = 0; i < size; i += 2) {
        tmp = buffer[i];
        buffer[i] = buffer[i + 1];
        buffer[i + 1] = tmp;
    }
}

static stdret_t get_buff_size(const char *devpath, int *size) {
    FILE *filp          = NULL;
    stdret_t ret        = STD_NOT_OK;
    char filepath[64]   = {0};

    /* The buffer size is way too big for now. Consider changing its length */
    char buffer[64]     = {0};

    /* Set up the filename by concatenating device name with subpath. */
    strcpy(filepath, devpath);
    strcat(filepath, BUFF_LEN_SUBPATH);

    filp = fopen(filepath, "r");
    if (NULL == filp) {
        fprintf(stderr, "ERROR: Error opening the file: %s\n", filepath);
        return ret;
    }

    fread(buffer, sizeof(char), sizeof(buffer), filp);
    fclose(filp);

    *size = atoi(buffer);

    ret = STD_OK;
    return ret;
}

static stdret_t hexdump_to_file(void *buffer, const int size, const habdev_t *habdev) {
    stdret_t ret = STD_NOT_OK;
    u8 byte = 0;
    u8 *data = (u8 *) buffer;
    FILE *filp = NULL;

    filp = fopen(habdev->log_path, "a+");
    if (NULL == filp) {
        fprintf(stderr, "ERROR: Error opening the file: %s\n", habdev->log_path);
        return ret;
    }

    for (usize i = 0; i < size; i++) {
        for (int j = 0; j < HEXDUMP_RECORD_LEN; j++) {
            byte = data[j + i * HEXDUMP_RECORD_LEN];
    
            fprintf(filp, "%02x", byte);
            if (j % 2 != 0)
                fprintf(filp, " ");
        }
        fprintf(filp, "\n");
    }
    fclose(filp);

    ret = STD_OK;
    return ret;
}

/**********************************************************************************************************************
 * GLOBAL FUNCTION DEFINITION
 *********************************************************************************************************************/
stdret_t iiobuff_log2file(char *ubuff, const habdev_t *habdev) {
    FILE *filp = NULL;
    stdret_t ret = STD_NOT_OK;
    const char *filepath = habdev->buff_path;
    int size = 0;

    ret = get_buff_size(habdev->dev_path, &size);
    
    filp = fopen(filepath, "r+");
    if (NULL == filp) {
        fprintf(stderr, "ERROR: Could not open the file %s.\n", filepath);
        return ret;
    }

    fread(ubuff, sizeof(char), size * HEXDUMP_RECORD_LEN, filp);
    fclose(filp);

    flip_nibbles(ubuff, size * HEXDUMP_RECORD_LEN);

    ret = hexdump_to_file(ubuff, size, habdev);
    if (STD_NOT_OK == ret) {
        return ret;
    }

    ret = STD_OK;
    return ret;
}