#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "hab_device.h"

#define IIO_BUFF_DEVFS_PATH "/dev/iio:"
#define IIO_DEV_SYSFS_PATH  "/sys/bus/iio/devices/iio:"
#define IIO_DEV_BASENAME    "device"

#define HAB_DATASTORAGE_PATH "/media/hab_flight_data/"

static habdev_t *habdev_list[64];
static usize habdev_count;

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

    to_char(idx, dev_index);
    strcat(dev_name, dev_index);

    ret = create_path(habdev->dev_path, 2, IIO_DEV_SYSFS_PATH, dev_name);
    ret = create_path(habdev->buff_path, 2, IIO_BUFF_DEVFS_PATH, dev_name);

    /* Read the device driver name (that is, the module name) */
    ret = create_path(path_buff, 2, habdev->dev_path, "/name");
    ret = read_file(path_buff, dev_name, sizeof(dev_name), MOD_R);

    if (STD_OK == ret)
        ret = create_path(habdev->log_path, 2, HAB_DATASTORAGE_PATH, dev_name);

    return ret;
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