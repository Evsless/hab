#ifndef __HAB_DEVICE_H__
#define __HAB_DEVICE_H__

#include <stdlib.h>
#include <stdbool.h>

#include "stdtypes.h"
#include "hab_trig.h"
#include "hab_device_types.h"

habdev_t *habdev_alloc(void);
habdev_t *habdev_get(const u32 idx);
void habdev_ev_set(habdev_t *habdev, ev_t *event);

stdret_t habdev_register(habdev_t *dev, u32 idx);

void habdev_free(habdev_t *dev);


#endif