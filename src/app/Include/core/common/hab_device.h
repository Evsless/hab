#ifndef __HAB_DEVICE_H__
#define __HAB_DEVICE_H__

#include <stdlib.h>
#include <stdbool.h>

#include "stdtypes.h"
#include "hab_trig.h"
#include "hab_device_types.h"

habdev_t *habdev_alloc(void);
habdev_t *habdev_get(const u32 idx);

stdret_t habdev_register(habdev_t *dev, u32 idx);

void habdev_getLogPath(const habdev_t *habdev, char *buff, usize size);
void habdev_getDevPath(const habdev_t *habdev, char *buff, usize size);

void habdev_preinit(void);
void habdev_postinit(void);

void habdev_free(habdev_t *dev);


#endif