#ifndef __HAB_DEVICE_H__
#define __HAB_DEVICE_H__

#include <stdlib.h>
#include <stdbool.h>

#include "stdtypes.h"
#include "hab_trig.h"
#include "iio_buffer_types.h"

typedef struct {
    u8 id;
    char dev_path[64];
    char buff_path[32];
    char log_path[64];
    iiobuff_format_t buff_format;
    habtrig_t *trig;
} habdev_t;


habdev_t *habdev_alloc(void);
stdret_t habdev_register(habdev_t *dev, u32 idx);
void habdev_free(habdev_t *dev);


#endif