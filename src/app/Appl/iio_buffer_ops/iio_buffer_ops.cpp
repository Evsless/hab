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
*       0.0.2               last modification: 14-04-2024                                                             *
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
#ifndef HAB_BUFF_CFG_PATH
# error "ERROR: Path to buffer configuration folder is not specified."
#endif

#define IRQ_LIST_PATH      "/proc/interrupts"
#define IRQ_TRIG_BASENAME  "irqtrig-"

#define HEXDUMP_RECORD_LEN 0x10
#define BUFF_LEN_SUBPATH   "/buffer/data_available"

#define BUFF_CFG_SCAN_ELEM "scan_elements/"
#define BUFF_CFG_LEN       "buffer/length"
#define BUFF_CFG_EN        "buffer/enable"

#define SETTING_GROUP 0
#define SETTING 1
#define SETTING_VALUE 2

/**********************************************************************************************************************
 * LOCAL TYPEDEFS DECLARATION
 *********************************************************************************************************************/

typedef struct {
    char setting_group;
    char setting[64];
    u32 val;
} cfgtoken_t;

/**********************************************************************************************************************
 * GLOBAL VARIABLES DECLARATION
 *********************************************************************************************************************/
static char data_buffer[4096];

/**********************************************************************************************************************
 * LOCAL FUNCTION DECLARATION
 *********************************************************************************************************************/
static void flip_nibbles(char *buffer, usize size);
static stdret_t get_buff_size(const habdev_t *habdev);
static stdret_t hexdump_to_file(void *buffer, const int size, const habdev_t *habdev);

static cfgtoken_t parse_cfg(const char *cfg);
static stdret_t write_cfg(const habdev_t *habdev, const cfgtoken_t *cfg);

static stdret_t find_irq_trigger(habdev_t *habdev);

/**********************************************************************************************************************
 * LOCAL FUNCTION DEFINITION
 *********************************************************************************************************************/
static cfgtoken_t parse_cfg(const char *cfg) {
    cfgtoken_t token;
    char buff[64] = {0};
    
    u8 token_idx  = 0;
    usize cfg_idx = 0;

    token.val = 1;
    while(get_word(cfg, &cfg_idx, buff, sizeof(buff)) >= 0) {
        switch (token_idx) {
            case SETTING_GROUP:
                token.setting_group = buff[0];
                break;
            case SETTING:
                token.setting[0] = '/'; /* Think of better way of handling that */
                strcpy(token.setting + 1, buff);
                break;
            case SETTING_VALUE:
                token.val = atoi(buff);
                break;
            default:
                break;
        }
        token_idx++;
    }

    return token;
}

static stdret_t write_cfg(const habdev_t *habdev, const cfgtoken_t *cfg) {
    stdret_t ret = STD_NOT_OK;
    char wr_buff[8] = {0};
    char setting_path[128] = {0};
    const char *setting_group = NULL;

    if ('c' == cfg->setting_group) {
        setting_group = "/scan_elements";
    } else if ('b' == cfg->setting_group) {
        setting_group = "/buffer";
    } else {
        /* ERROR HANDLING ?*/
    }

    ret = create_path(setting_path, 3, habdev->dev_path, setting_group, cfg->setting);
    to_char(cfg->val, wr_buff);
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
    char dev_name[32] = {0};
    char first_word[32] = {0};

    usize fpos = 0;
    usize lpos = 0;

    bool repeat = true;
    bool line_begin = true;

    /* Read device name. Same name is used in /proc/interrupts list. */
    ret = create_path(line, 2, habdev->dev_path, "/name");
    ret = read_file(line, dev_name, sizeof(dev_name), MOD_R);
    CROP_NEWLINE(dev_name, strlen(dev_name));

    while(get_line(IRQ_LIST_PATH, &fpos, line, sizeof(line)) >= 0 && repeat) {
        while(get_word(line, &lpos, word, sizeof(word)) >= 0 && repeat) {
            if (line_begin) {
                line_begin = false;
                strcpy(first_word, word);
            }

            if (0 == str_compare(dev_name, word)) {
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
    FILE *filp = NULL;
    stdret_t ret = STD_NOT_OK;
    usize cfg_pos = 0;
    cfgtoken_t token;

    if (T_HRTIM != habdev->trig->type) {
        ret = find_irq_trigger(habdev);
        if (STD_OK == ret)
            fprintf(stdout, "INFO: irq-trigger %s for device found.\n", habdev->trig->name);
    }

    char filepath_buff[128] = {0};
    char append_buff[64] = {0};

    /* Setting up a trigger for a buffer */
    ret = create_path(filepath_buff, 3, habdev->dev_path, "/trigger", "/current_trigger");
    ret = write_file(filepath_buff, habdev->trig->name, sizeof(habdev->trig->name), MOD_W);

    /* Reading the device name */
    ret = create_path(filepath_buff, 2, habdev->dev_path, "/name");
    ret = read_file(filepath_buff, append_buff, sizeof(append_buff), MOD_R);

    memset(filepath_buff, 0, sizeof(filepath_buff));
    ret = create_path(filepath_buff, 2, HAB_BUFF_CFG_PATH, append_buff);
    
    while (get_line(filepath_buff, &cfg_pos, append_buff, sizeof(append_buff)) >= 0) {
        token = parse_cfg(append_buff);
        ret   = write_cfg(habdev, &token);
    }

    return ret;
}

stdret_t iiobuff_log2file(const habdev_t *habdev) {
    stdret_t ret = STD_NOT_OK;
    int size = 0;
    char blen[8];
    char buffer[128];

    ret = create_path(buffer, 2, habdev->dev_path, BUFF_LEN_SUBPATH);
    ret = read_file(buffer, blen, sizeof(buffer), MOD_R);
    size = atoi(blen);

    ret = read_file(habdev->buff_path, data_buffer, size * HEXDUMP_RECORD_LEN, MOD_RW);
    flip_nibbles(data_buffer, size * HEXDUMP_RECORD_LEN);

    ret = hexdump(habdev->log_path, data_buffer, size);

    ret = STD_OK;
    return ret;
}