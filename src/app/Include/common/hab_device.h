#ifndef __HAB_DEVICE_H__
#define __HAB_DEVICE_H__

#include <stdlib.h>
#include <stdbool.h>

#include "stdtypes.h"
#include "hab_trig.h"
#include "iio_buffer_types.h"

typedef struct {
    const char *dev_path;
    const char *buff_path;
    const char *log_path;
    iiobuff_format_t buff_format;
    habtrig_t *trig;
} habdev_t;


habdev_t *habdev_init(void);
void habdev_free(habdev_t *dev);


#endif