/**********************************************************************************************************************
* hab_device.cpp                                                                                                      *
***********************************************************************************************************************
* DESCRIPTION :                                                                                                       *
*       High altitude baloon onboard PC attached devices description. Includes APIs                                   *
*       for managing attached devices parameters.                                                                     *
*                                                                                                                     *
* PUBLIC FUNCTIONS :                                                                                                  *
*       habdev_t            *habdev_alloc(void)                                                                       *
*       stdret_t            habdev_register(habdev_t *habdev, u32 idx)                                                *
*       void                habdev_ev_set(habdev_t *habdev, ev_t *event)                                              *
*       void                habdev_trig_set(habdev_t *habdev, habtrig_t *trig)                                        *
*       habdev_t            *habdev_get(const u32 idx)                                                                *
*       void                habdev_free(habdev_t *habdev)                                                             *
*                                                                                                                     *
* AUTHOR :                                                                                                            *
*       Yahor Yauseyenka    email: yahoryauseyenka@gmail.com                                                          *
*                                                                                                                     *
* VERSION                                                                                                             *
*       0.0.4               last modification: 18-04-2024                                                             *
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
#include "hab_device.h"

/**********************************************************************************************************************
 *  MACRO
 *********************************************************************************************************************/
#define IIO_BUFF_DEVFS_PATH  "/dev/iio:"

#define IIO_DEV_SYSFS_PATH   "/sys/bus/iio/devices/iio:"
#define IIO_DEV_NAME_SUBPATH "/name"
#define IIO_DEV_ADDR_SUBPATH "/of_node/reg"

#define IIO_DEV_BASENAME    "device"

#define HAB_DATASTORAGE_PATH "/media/hab_flight_data/"

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
    char dev_name[16] = IIO_DEV_BASENAME;
    char dev_index[8] = {0};
    char path_buff[64] = {0};

    habdev->index = idx;

    sprintf(dev_name, "%s%d", IIO_DEV_BASENAME, idx);

    ret = create_path(habdev->path.dev_path, 2, IIO_DEV_SYSFS_PATH, dev_name);
    ret = create_path(habdev->path.buff_path, 2, IIO_BUFF_DEVFS_PATH, dev_name);

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