#ifndef __EVENT_H__
#define __EVENT_H__

#include "event_types.h"
#include "hab_device_types.h"

ev_t *event_alloc(void);
stdret_t ev_setup(habdev_t *habdev);

#endif /* __EVENT_H__ */