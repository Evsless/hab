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
*       0.0.4               last modification: 20-04-2024                                                             *
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

#include "utils.h"
#include "iio_buffer_ops.h"

/**********************************************************************************************************************
 *  MACRO
 *********************************************************************************************************************/
#ifndef HAB_DEV_CFG_PATH
# error "ERROR: Path to buffer configuration folder is not specified."
#endif

#define IRQ_LIST_PATH         "/proc/interrupts"
#define BUFF_DATA_RDY_SUBPATH "buffer/data_available"

#define IRQ_TRIG_BASENAME  "irqtrig-"

/**********************************************************************************************************************
 * LOCAL TYPEDEFS DECLARATION
 *********************************************************************************************************************/
 

/**********************************************************************************************************************
 * GLOBAL VARIABLES DECLARATION
 *********************************************************************************************************************/
static char data_buffer[4096];

/**********************************************************************************************************************
 * LOCAL FUNCTION DECLARATION
 *********************************************************************************************************************/
static void flip_nibbles(char *buffer, usize size);
static stdret_t find_irq_trigger(habdev_t *habdev);


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

static stdret_t find_irq_trigger(habdev_t *habdev) {
    stdret_t ret;
    char line[128] = {0};
    char word[64]  = {0};
    char first_word[32] = {0};

    usize fpos = 0;
    usize lpos = 0;

    bool repeat = true;
    bool line_begin = true;

    while(get_line(IRQ_LIST_PATH, &fpos, line, sizeof(line)) >= 0 && repeat) {
        while(get_word(line, &lpos, word, sizeof(word)) >= 0 && repeat) {
            if (line_begin) {
                line_begin = false;
                strcpy(first_word, word);
            }

            if (0 == str_compare(habdev->path.dev_name, word)) {
                repeat = false;
            }
        }
        lpos = 0;
        line_begin = true;
    }

    /* If while loop was interrupted - device entry found. */
    if (!repeat) {
        CROP_LAST_CHAR(first_word, strlen(first_word));
        snprintf(habdev->trig->name, sizeof(habdev->trig->name), "%s%s", IRQ_TRIG_BASENAME, first_word);
        habdev->trig->type = T_IRQ;
    
        ret = STD_OK;
    } else {
        ret = STD_NOT_OK;
    }
    return ret;
}

/**********************************************************************************************************************
 * GLOBAL FUNCTION DEFINITION
 *********************************************************************************************************************/
int iiobuff_log2file(const habdev_t *habdev, const char *append, u8 *data_cpy) {
    stdret_t ret = STD_NOT_OK;
    int size = 0;
    char blen[8] = {0};
    char log_path[64] = {0};
    char dev_path[128] = {0};

    habdev_getDevPath(habdev, dev_path, sizeof(dev_path));
    strcat(dev_path, BUFF_DATA_RDY_SUBPATH);

    ret = read_file(dev_path, blen, sizeof(dev_path), MOD_R);
    size = atoi(blen);

    if (size > 0) {
        ret = read_file(habdev->path.buffer[0], data_buffer, size * HEXDUMP_RECORD_LEN, MOD_RW);
        if (NULL != data_cpy)
            memcpy(data_cpy, data_buffer, sizeof(data_buffer));

        flip_nibbles(data_buffer, size * HEXDUMP_RECORD_LEN);

        habdev_getLogPath(habdev, log_path, sizeof(log_path));
        ret = hexdump(log_path, data_buffer, size, append);   
    }

    return (ret == STD_OK) ? size * HEXDUMP_RECORD_LEN : -1;
}

int iiobuff_extract_data(data_format_t format, s64 *dst, const u8 *src, const usize size) {
    usize batch_size = 0;
    s64 chan_val = 0;
    int dst_size = 0, chan_cnt = 0, pad = 0;
    int data_remain = size;

    for (int i = 0; i < format.chan_num; i++)
        batch_size += format.storagebits[i] / BYTE;
    
    if (false == format.ts_en && (batch_size < 16))
        batch_size = 16;
    
    /* Use case applicable for buffers such as icm20x */
    // batch_size = min(n , HEXDUMP_RECORD_LEN);

    for (usize i = 0; i < size; i += HEXDUMP_RECORD_LEN) {
        for (usize j = 0; j < batch_size && data_remain > 0; ) {
            pad = 0;
            chan_cnt %= format.chan_num;
            /**
             * TBD Affter adding a setting we could also log a TS.
             */
            // if ((format.chan_num - 1 == chan_cnt) && format.ts_en)
            //     pad = HEXDUMP_RECORD_LEN - batch_size;
            
            chan_val = merge_bytes(src + i + j + pad, format.storagebits[chan_cnt]);

            /**
             * For a buffer where ammount of data pushed does not devide without a remainder,
             * an amount of bytes to be proccessed needs to be tracked.
             * EXAMPLE - icm20x accelerometer. 
             *     Single portion of data is 6 bytes. It means that hexdump line contains 2 full readouts.
             *     The rest of thirt readout will be in the 0x10 hexdump line. Without tracking the remainder,
             *     14 bytes in 0x10 line will unnececarilly stored. 
             */
            data_remain -= format.storagebits[chan_cnt] / BYTE;
            j += format.storagebits[chan_cnt++] / BYTE;
            dst[dst_size++] = chan_val;
        }
    }

    return dst_size;
}

/***********************************************************************************************************************
 * END OF FILE
 **********************************************************************************************************************/
 