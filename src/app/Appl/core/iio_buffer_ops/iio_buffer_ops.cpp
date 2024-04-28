/**********************************************************************************************************************
* iio_buffer_ops.cpp                                                                                                  *
***********************************************************************************************************************
* DESCRIPTION :                                                                                                       *
*       Industrial I/O control routines. Routines are used when handling the data stored inside                       *
*       iio triggered buffer.                                                                                         *
*                                                                                                                     *
* PUBLIC FUNCTIONS :                                                                                                  *
*       stdret_t            iiobuff_log2file(char *ubuff, const habdev_t *habdev)                                     *
*       stdret_t            iiobuff_setup(habdev_t *habdev)                                                           *
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
#include "parser.h"
#include "iio_buffer_ops.h"

/**********************************************************************************************************************
 *  MACRO
 *********************************************************************************************************************/
#ifndef HAB_BUFF_CFG_PATH
# error "ERROR: Path to buffer configuration folder is not specified."
#endif

#define IRQ_LIST_PATH         "/proc/interrupts"
#define IIO_BUFF_DEVFS_PATH   "/dev/iio:"
#define BUFF_CH_SUBPATH       "/scan_elements/"
#define BUFF_LEN_SUBPATH      "/buffer/length"
#define BUFF_EN_SUBPATH       "/buffer/enable"
#define BUFF_DATA_RDY_SUBPATH "/buffer/data_available"

#define IIO_DEV_BASENAME    "device"
#define IRQ_TRIG_BASENAME  "irqtrig-"

#define IIO_BUFF_PATH_SIZE 32U
#define HEXDUMP_RECORD_LEN 0x10

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
static stdret_t write_cfg(const habdev_t *habdev, const cfgtoken_t *cfg);

static int get_chan_storagebits(const habdev_t *habdev, const char *chan);

/**********************************************************************************************************************
 * LOCAL FUNCTION DEFINITION
 *********************************************************************************************************************/
static int get_chan_storagebits(const habdev_t *habdev, const char *chan) {
    int ret = 0;
    int cnt = 0;

    char tmp = '\0';
    char cfg_path[128] = {0};
    char ch_format[32] = {0};

    snprintf(cfg_path, sizeof(cfg_path), "%s/%s/%s",
            habdev->path.dev_path, "scan_elements", chan);
    strcpy(cfg_path + (strlen(cfg_path) - 2), "type");

    read_file(cfg_path, ch_format, sizeof(ch_format), MOD_R);
    CROP_NEWLINE(ch_format, strlen(ch_format))

    for (cnt = 0; tmp != '/'; cnt++)
        tmp = ch_format[cnt];

    for (; ch_format[cnt] != '>'; cnt++)
        ret = ret * 10 + (ch_format[cnt] - '0');

    return ret;
}

static stdret_t write_cfg(const habdev_t *habdev, const cfgtoken_t *cfg) {
    stdret_t ret = STD_NOT_OK;
    char wr_buff[8] = {0};
    char setting_path[128] = {0};

    if (cfg->cfg_type >= CFG_CH_IN_TS) {
        ret = create_path(setting_path, 3, habdev->path.dev_path, BUFF_CH_SUBPATH, parser_get_chan(cfg->cfg_type));
    } else if (CFG_BUFF_LEN == cfg->cfg_type) {
        ret = create_path(setting_path, 2, habdev->path.dev_path, BUFF_LEN_SUBPATH);
    } else if (CFG_BUFF_EN == cfg->cfg_type) {
        ret = create_path(setting_path, 2, habdev->path.dev_path, BUFF_EN_SUBPATH);
    } else {
        fprintf(stderr, "ERROR: Unknown setting configuration for buffer. Code: %d\n", cfg->cfg_type);
        return STD_NOT_OK;
    }

    snprintf(wr_buff, sizeof(wr_buff), "%d", atoi(cfg->val));
    ret = write_file(setting_path, wr_buff, sizeof(wr_buff), MOD_W);

    return ret;
}

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
        ret = create_path(habdev->trig->name, 2, IRQ_TRIG_BASENAME, first_word);
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
stdret_t iiobuff_setup(habdev_t *habdev) {
    stdret_t ret = STD_NOT_OK;
    usize cfg_pos = 0;
    cfgtoken_t token;

    char file_path[128] = {0};
    char line[64] = {0};

    /* If buffer is not triggered by hrtim, look for an IRQ trigger */
    if (T_HRTIM != habdev->trig->type) {
        ret = find_irq_trigger(habdev);
        if (STD_OK == ret)
            fprintf(stdout, "INFO: irq-trigger %s for device found.\n", habdev->trig->name);
    }

    /* Setting up a trigger for a buffer */
    ret = create_path(file_path, 3, habdev->path.dev_path, "/trigger", "/current_trigger");
    ret = write_file(file_path, habdev->trig->name, sizeof(habdev->trig->name), MOD_W);

    ret = create_path(file_path, 2, HAB_BUFF_CFG_PATH, habdev->path.dev_name);
    // while (get_line(file_path, &cfg_pos, line, sizeof(line)) >= 0) {
    for (int i = 0; habdev->conf[i].cfg_type != -1; i++) {
        token = habdev->conf[i];

        /* Allocate a path to buffer only if it is enabled */
        if (CFG_BUFF_EN == token.cfg_type) {
           habdev->path.dev_data[0] = (char *)malloc(IIO_BUFF_PATH_SIZE);
            if (habdev->path.dev_data[0]) {
                snprintf(habdev->path.dev_data[0], IIO_BUFF_PATH_SIZE, 
                        "%s%s%d", IIO_BUFF_DEVFS_PATH, IIO_DEV_BASENAME, habdev->index);
            } else {
                fprintf(stderr, "ERROR: Memory allocation error. Alocation for iio %s device\n", habdev->path.dev_name);
                return STD_NOT_OK;
            } 
        }

        /* Write config when not configuring the CFG_DEV_TYPE */
        if ((token.cfg_type >> 2) != 1)
            ret  = write_cfg(habdev, &token);

        /* Setup buffer layout (ammount of data and its format) */
        if ((token.cfg_type >> 7) == 1) {
            if (token.cfg_type != CFG_CH_IN_TS)
                habdev->df.storagebits[habdev->df.chan_num++] = get_chan_storagebits(habdev, parser_get_chan(token.cfg_type));
            else
                habdev->df.ts_en = true;
        }
    }
    return ret;
}

int iiobuff_log2file(const habdev_t *habdev, const char *append, u8 *data_cpy) {
    stdret_t ret = STD_NOT_OK;
    int size = 0;
    char blen[8];
    char buffer[128];

    ret = create_path(buffer, 2, habdev->path.dev_path, BUFF_DATA_RDY_SUBPATH);
    ret = read_file(buffer, blen, sizeof(buffer), MOD_R);
    size = atoi(blen);

    ret = read_file(habdev->path.dev_data[0], data_buffer, size * HEXDUMP_RECORD_LEN, MOD_RW);
    if (NULL != data_cpy)
        memcpy(data_cpy, data_buffer, sizeof(data_buffer));

    flip_nibbles(data_buffer, size * HEXDUMP_RECORD_LEN);
    ret = hexdump(habdev->path.log_path, data_buffer, size, append);

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
            // printf("%ld ", chan_val);
        }
        // printf("\n");
    }

    return dst_size;
}

/***********************************************************************************************************************
 * END OF FILE
 **********************************************************************************************************************/
 