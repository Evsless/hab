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
*       void                habdev_ev_set(habdev_t *habdev, ev_t *event)                                              *
*       void                habdev_trig_set(habdev_t *habdev, habtrig_t *trig)                                        *
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
#include "parser.h"
#include "hab_device.h"

/**********************************************************************************************************************
 *  MACRO
 *********************************************************************************************************************/
#define IIO_DEV_SYSFS_PATH   "/sys/bus/iio/devices/iio:"
#define IIO_BUFF_DEVFS_PATH  "/dev/iio:"
#define HAB_DATASTORAGE_PATH "/media/hab_flight_data/"
#define IIO_DEV_NAME_SUBPATH "/name"
#define IIO_DEV_ADDR_SUBPATH "/of_node/reg"

#define IIO_DEV_BASENAME "device"

/**********************************************************************************************************************
 * LOCAL TYPEDEFS DECLARATION
 *********************************************************************************************************************/
 

/**********************************************************************************************************************
 * GLOBAL VARIABLES DECLARATION
 *********************************************************************************************************************/
static habdev_t *habdev_list[64];
static usize habdev_count;

/**********************************************************************************************************************
 * LOCAL FUNCTION DECLARATION
 *********************************************************************************************************************/
static u8 get_dev_address(const char *of_node_reg);

/**********************************************************************************************************************
 * LOCAL FUNCTION DEFINITION
 *********************************************************************************************************************/
static u8 get_dev_address(const char *of_node_reg) {
    u8 ret = 0;

    for (usize cnt = 0; ret == 0; cnt++) {
        ret = (u8)of_node_reg[cnt];
    }
    
    return ret;
}

/**********************************************************************************************************************
 * GLOBAL FUNCTION DEFINITION
 *********************************************************************************************************************/
habdev_t *habdev_alloc(void) {
    habdev_t *habdev = NULL;

    habdev = (habdev_t *)malloc(sizeof(habdev_t));

    habdev->id = habdev_count;
    habdev_list[habdev_count++] = habdev;

    return habdev;
}

stdret_t habdev_register(habdev_t *habdev, u32 idx) {
    stdret_t ret = STD_NOT_OK;
    char dev_name[16] = {0};
    char dev_index[8] = {0};
    char path_buff[64] = {0};

    habdev->index = idx;

    sprintf(dev_name, "%s%d", IIO_DEV_BASENAME, idx);

    ret = create_path(habdev->path.dev_path, 2, IIO_DEV_SYSFS_PATH, dev_name);

    /* Read the device driver name (that is, the module name) */
    ret = create_path(path_buff, 2, habdev->path.dev_path, IIO_DEV_NAME_SUBPATH);
    ret = read_file(path_buff, dev_name, sizeof(dev_name), MOD_R);
    CROP_NEWLINE(dev_name, strlen(dev_name));

    /* Read the device address (that will be a part of device name) */
    ret = create_path(path_buff, 2, habdev->path.dev_path, IIO_DEV_ADDR_SUBPATH);
    ret = read_file(path_buff, dev_index, sizeof(dev_name), MOD_R);

    /* Setup a device name (used when reading configs) */
    u8 addr = get_dev_address(dev_index);
    sprintf(habdev->path.dev_name, "%s-%02x", dev_name, addr);

    if (STD_OK == ret) {
        ret = create_path(habdev->path.log_path, 2, HAB_DATASTORAGE_PATH, habdev->path.dev_name);
    }
    
    return ret;
}

stdret_t habdev_iio_data_setup(habdev_t *habdev) {
    stdret_t ret = STD_NOT_OK;
    cfgtoken_t token;
    dev_type_t dev_type = DEV_UNKNOWN;

    int alloc_size = 0;
    usize file_pos = 0;
    usize data_cnt = 0;
    char dev_data_buff[256] = {0};
    char file_path[128] = {0};
    char line[64] = {0};
    const char *chan = NULL;

    snprintf(file_path, sizeof(file_path), "%s%s", HAB_BUFF_CFG_PATH, habdev->path.dev_name);
    while (get_line(file_path, &file_pos, line, sizeof(line)) >= 0) {
        token = parser_parse(line);

        if ((token.cfg_type >> 2) == 1)
            dev_type = (dev_type_t) (token.cfg_type & 0x03);
        
        if ((token.cfg_type >> 7) == 1) {
            chan = parser_get_chan(token.cfg_type);

            switch (dev_type) {
                case DEV_IIO:
                    alloc_size = sizeof(habdev->path.dev_path) + sizeof(chan) + 1;
                    snprintf(dev_data_buff, alloc_size, "%s/%s", habdev->path.dev_path, chan);
                    break;
                case DEV_DEFAULT:
                    alloc_size = sizeof(token.val);
                    snprintf(dev_data_buff, alloc_size, "%s", token.val);
                    break;
                default:
                    break;
            } 

            if ((NULL != chan && dev_type == DEV_IIO) || (token.val[0] != '\0' && dev_type == DEV_DEFAULT) ) {
                habdev->path.dev_data[data_cnt] = (char *)malloc(alloc_size);
                if (NULL == habdev->path.dev_data[data_cnt]) {
                    fprintf(stderr, "ERROR: Error when allocating memory for device data path. Device: %s\n", habdev->path.dev_name);
                    return STD_NOT_OK;
                }

                snprintf(habdev->path.dev_data[data_cnt++], alloc_size, "%s", dev_data_buff);
            } else {
                fprintf(stderr, "ERROR: Wrong device configuration. Config file: %s\n", file_path);
                return STD_NOT_OK;
            }

        }
    }

    return ret;
}

void habdev_ev_set(habdev_t *habdev, ev_t *event) {
    if (NULL != event && NULL != habdev)
        habdev->event = event;

    uv_handle_set_data(event->handle, habdev);
}

void habdev_trig_set(habdev_t *habdev, habtrig_t *trig) {
    if (habdev && trig)
        habdev->trig = trig;
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
