/**********************************************************************************************************************
* hab_device.cpp                                                                                                      *
***********************************************************************************************************************
* DESCRIPTION :                                                                                                       *
*       High altitude baloon onboard PC attached devices description. Includes APIs                                   *
*       for managing attached devices parameters.                                                                     *
*                                                                                                                     *
* PUBLIC FUNCTIONS :                                                                                                  *
*       habdev_t*           habdev_alloc(void)                                                                        *
*       stdret_t            habdev_register(habdev_t *habdev, u32 idx)                                                *
*       habdev_t*           habdev_get(const u32 idx)                                                                 *
*       void                habdev_free(habdev_t *habdev)                                                             *
*                                                                                                                     *
* AUTHOR :                                                                                                            *
*       Yahor Yauseyenka    email: yahoryauseyenka@gmail.com                                                          *
*                                                                                                                     *
* VERSION                                                                                                             *
*       0.0.6               last modification: 20-04-2024                                                             *
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

#include "utils.h"
#include "event.h"
#include "callback.h"
#include "hab_device.h"

#include "llist.h"
#include "cfg_tree.h"

/**********************************************************************************************************************
 *  MACRO
 *********************************************************************************************************************/
#define IIO_DEV_SYSFS_PATH   "/sys/bus/iio/devices/iio:device"
#define IIO_BUFF_DEVFS_PATH  "/dev/iio:device"
#define HAB_DATASTORAGE_PATH "/media/hab_flight_data/"
#define IIO_DEV_NAME_SUBPATH "/name"
#define IIO_DEV_SCAN_EL_SUBPATH "scan_elements/"

/**********************************************************************************************************************
 * LOCAL TYPEDEFS DECLARATION
 *********************************************************************************************************************/
 

/**********************************************************************************************************************
 * GLOBAL VARIABLES DECLARATION
 *********************************************************************************************************************/
static habdev_t *habdev_list[64];
static usize habdev_count;

static const char *dev_names[] = HAB_DEV_NAME;
static const s8 trig_lut[]     = TRIG_LUT;

static char config_buff[128];

extern CALLBACK (*ev_tim_callback_list[64])(uv_timer_t *handle);

/**********************************************************************************************************************
 * LOCAL FUNCTION DECLARATION
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * LOCAL FUNCTION DEFINITION
 *********************************************************************************************************************/
static stdret_t add_channel(char **habdev_ch, const char *ch) {
    int alloc_size = 0;
    alloc_size = strlen(ch) + 1;

    *habdev_ch = (char *) malloc(alloc_size);
    if (NULL == *habdev_ch) {
        fprintf(stderr, "ERROR: Error allocating data channel for device. Channel: %s\n", ch);
        return STD_NOT_OK;
    }
    snprintf(*habdev_ch, alloc_size, "%s", ch);

    return STD_OK;
}

static stdret_t get_storagebits(habdev_t *habdev, const char *chan) {
    stdret_t retval = STD_NOT_OK;
    int bits = 0;
    int cnt = 0;

    char tmp = '\0';
    char cfg_path[128] = {0};
    char ch_format[32] = {0};
    char dev_path[64] = {0};

    habdev_getDevPath(habdev, dev_path, sizeof(dev_path));
    snprintf(cfg_path, sizeof(cfg_path), "%s%s%s", dev_path, IIO_DEV_SCAN_EL_SUBPATH, chan);
    strcpy(cfg_path + (strlen(cfg_path) - 2), "type");

    retval = read_file(cfg_path, ch_format, sizeof(ch_format), MOD_R);
    if (STD_NOT_OK == retval)
        return STD_NOT_OK;
    CROP_NEWLINE(ch_format, strlen(ch_format))

    for (cnt = 0; tmp != '/'; cnt++)
        tmp = ch_format[cnt];
    
    for (; ch_format[cnt] != '>'; cnt++)
        bits = bits * 10 + (ch_format[cnt] - '0');

    habdev->df.storagebits[habdev->df.chan_num++] = bits;
    if (0 == str_compare(chan, "in_timestamp_en"))
        habdev->df.ts_en = true;

    return STD_OK;
}


static stdret_t write_config(const habdev_t *habdev, node_t *node, int cfg) {
    stdret_t retval      = STD_OK;
    const char *cfg_path = NULL;
    char path_buff[128]  = {0};
    char dev_path[64]    = {0};
    int child_cnt        = 0;

    cfg |= cfgtree_getCfgReg(node->type);
    habdev_getDevPath(habdev, dev_path, sizeof(dev_path));

    switch (cfg & 0xFFFF) {
    case CFGTREE_BUFF_CONFIG:
        snprintf(path_buff, sizeof(path_buff), "%s%s", dev_path, "trigger/current_trigger");
        retval = write_file(path_buff, habdev->trig->name, sizeof(habdev->trig->name), MOD_W);
        break;
    case CFGTREE_BUFF_CHAN_NAME_CONFIG:
        snprintf(config_buff, sizeof(config_buff), "%s%s", IIO_DEV_SCAN_EL_SUBPATH, node->val);
        break;
    case CFGTREE_BUFF_CHAN_VAL_CONFIG:
        snprintf(path_buff, sizeof(path_buff), "%s%s", dev_path, config_buff);
        retval = write_file(path_buff, node->val, sizeof(node->val), MOD_W);
        memset(config_buff, 0, sizeof(config_buff));
        break;
    case CFGTREE_BUFF_LEN_CONFIG:
        snprintf(path_buff, sizeof(path_buff), "%s%s", dev_path, "buffer/length");
        retval = write_file(path_buff, node->val, sizeof(node->val), MOD_W);
        break;
    case CFGTREE_BUFF_ENABLE:
        snprintf(path_buff, sizeof(path_buff), "%s%s", dev_path, "buffer/enable");
        retval = write_file(path_buff, node->val, sizeof(node->val), MOD_W);
        break;
    case CFGTREE_CHAN_NAME_CONFIG:
        snprintf(config_buff, sizeof(config_buff), "%s", node->val);
        break;
    case CFGTREE_CHAN_VAL_CONFIG:
        snprintf(path_buff, sizeof(path_buff), "%s%s", dev_path, config_buff);
        retval = write_file(path_buff, node->val, sizeof(node->val), MOD_W);
        memset(config_buff, 0, sizeof(config_buff));
        break;
    default:
        break;
    }

    if (STD_NOT_OK == retval)
        return STD_NOT_OK;

    while (child_cnt < node->child_num) {
        retval = write_config(habdev, node->child[child_cnt++], cfg);
    }

    return retval;
}

static stdret_t save_config(habdev_t *habdev, node_t *node, int cfg) {
    stdret_t retval = STD_OK;

    int child_cnt = 0;
    char buff[64] = {0};

    cfg |= cfgtree_getCfgReg(node->type);

    /* A device type shall not be considered (0xFFFF mask) */
    switch (cfg & 0xFFFF) {
    case CFGTREE_BUFF_CONFIG:
        snprintf(buff, sizeof(buff), "%s%d", IIO_BUFF_DEVFS_PATH, habdev->index);
        retval = add_channel(&habdev->path.buffer[habdev->buffer_num++], buff);
        break;
    case CFGTREE_CHAN_NAME_CONFIG:
        snprintf(buff, sizeof(buff), "%s", node->val);
        retval = add_channel(&habdev->path.channel[habdev->channel_num++], buff);
        break;
    case CFGTREE_BUFF_CHAN_NAME_CONFIG:
        retval = get_storagebits(habdev, node->val);
        break;
    case CFGTREE_EVENT:
        habdev->event = event_alloc();
        if (NULL == habdev->event)
            retval = STD_NOT_OK;
        habdev->event->tim_cb = ev_tim_callback_list[habdev->index];
        uv_handle_set_data(habdev->event->handle, habdev);
        break;
    case CFGTREE_EVENT_TIM_TO_CONFIG:
        habdev->event->hcfg.tim_ev.tim_to = atoi(node->val);
        break;
    case CFGTREE_EVENT_TIM_REP_CONFIG:
        habdev->event->hcfg.tim_ev.tim_rep = atoi(node->val);
        break;
    default:
        break;   
    }

    if (STD_NOT_OK == retval)
        return STD_NOT_OK;

    while(child_cnt < node->child_num) {
        save_config(habdev, node->child[child_cnt++], cfg);
    }

    return STD_OK;
}


/**********************************************************************************************************************
 * GLOBAL FUNCTION DEFINITION
 *********************************************************************************************************************/
void habdev_preinit(void) {
    cfgtree_initDfa();
}


void habdev_postinit(void) {
    cfgtree_freeDfa();
}

void habdev_getDevPath(const habdev_t *habdev, char *buff, usize size) {
    memset(buff, 0, size);

    switch(habdev->dev_type) {
    case DEV_IIO:
    case DEV_IIO_BUFF:
        snprintf(buff, size, "%s%d/", IIO_DEV_SYSFS_PATH, habdev->index);
        break;
    default:
        break;
    }
}

void habdev_getLogPath(const habdev_t *habdev, char *buff, usize size) {
    memset(buff, 0, size);
    snprintf(buff, size, "%s%s", HAB_DATASTORAGE_PATH, habdev->path.dev_name);
}

habdev_t *habdev_alloc(void) {
    habdev_t *habdev = NULL;

    habdev = (habdev_t *)malloc(sizeof(habdev_t));
    if (NULL == habdev){
        fprintf(stderr, "ERROR: Error allocating the memory for hab device.\n");
        return NULL;
    }

    memset(habdev, 0, sizeof(habdev));
    habdev->id = habdev_count;
    habdev_list[habdev_count++] = habdev;

    return habdev;
}

stdret_t habdev_register(habdev_t *habdev, u32 idx) {
    stdret_t retval = STD_NOT_OK;
    char path_buff[64]  = {0};
    char line[64]       = {0};
    usize file_pos      = 0;
    int readline_stat   = 0;

    habdev->index = idx;
    snprintf(habdev->path.dev_name, sizeof(habdev->path.dev_name), "%s", dev_names[habdev->index]);

    /* Parse the device configuration and save it */
    xml_line_t *xml_cfg = llist_init();
    snprintf(path_buff, sizeof(path_buff), "%s%s", HAB_BUFF_CFG_PATH, dev_names[habdev->index]);
    for (int i = 0; (readline_stat = get_line(path_buff, &file_pos, line, sizeof(line))) >= 0; i++)
        llist_push(xml_cfg, line);

    if (readline_stat == -2)
        return STD_NOT_OK;
    
    cfgtree_dfaReset();
    habdev->node = cfgtree_init(&xml_cfg);
    /* First entrance in the config xml file is the device config */
    habdev->dev_type = (dev_type_t)habdev->node->type;
    if (habdev->dev_type == DEV_IIO_BUFF)
        habdev->trig = habtrig_get(trig_lut[habdev->index]);

    retval = save_config(habdev, habdev->node, 0);
    if (STD_NOT_OK == retval) {
        fprintf(stderr, "ERROR: Error saving configuration for device: %s\n", habdev->path.dev_name);
        return STD_NOT_OK;
    }

    retval = write_config(habdev, habdev->node, 0);
    if (STD_NOT_OK == retval) {
        fprintf(stderr, "ERROR: Error writing configuration for device: %s\n", habdev->path.dev_name);
        return STD_NOT_OK;
    }

    llist_free(xml_cfg);

    return retval;
}


habdev_t *habdev_get(const u32 idx) {
    habdev_t *habdev = NULL;

    for (usize i = 0; i < ARRAY_SIZE(habdev_list); i++) {
        if (idx == habdev_list[i]->index) {
            habdev = habdev_list[i];
            break;
        }
    }
    return habdev;
}

void habdev_free(habdev_t *habdev) {
    free(habdev);
}

/***********************************************************************************************************************
 * END OF FILE
 **********************************************************************************************************************/
