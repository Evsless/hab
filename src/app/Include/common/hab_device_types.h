#ifndef __HAB_DEVICE_TYPES_H__
#define __HAB_DEVICE_TYPES_H__

#include "hab_trig.h"
#include "event_types.h"

typedef struct {
    char dev_name[16];
    char dev_path[64];
    char log_path[64];
    char *dev_data[16];
} hab_path_t;

typedef struct {
    u8 id;
    u8 index;
    hab_path_t path;
    habtrig_t *trig;
    ev_t *event;
} habdev_t;

#endif /* __HAB_DEVICE_TYPES_H__ */
