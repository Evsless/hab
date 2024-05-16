#ifndef __EVENT_H__
#define __EVENT_H__

#include "event_types.h"

ev_t *event_alloc(void);
ev_glob_t *event_allocGlobalEv(void);
stdret_t event_registerGlobalEv(ev_glob *ev_glob, const u8 index);

usize event_getGlobalNum(void);
ev_glob_t *event_getGlobalEv(const int id);

stdret_t event_addMeasuredDev(const int ev_glob_id, const int habdev_id);

#endif /* __EVENT_H__ */