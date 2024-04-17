#ifndef __HAB_DEVICE_H__
#define __HAB_DEVICE_H__

#include <stdlib.h>
#include <stdbool.h>

#include "event.h"
#include "stdtypes.h"
#include "hab_trig.h"
#include "iio_buffer_types.h"

typedef struct {
    char dev_name[16];
    char dev_path[64];
    char buff_path[32];
    char log_path[64];
} path_t;

typedef struct {
    u8 id;
    u8 index;
    path_t path;
    habtrig_t *trig;
    ev_t *event;
} habdev_t;


habdev_t *habdev_alloc(void);
habdev_t *habdev_get(const u32 idx);
stdret_t habdev_register(habdev_t *dev, u32 idx);
void habdev_free(habdev_t *dev);


#endif