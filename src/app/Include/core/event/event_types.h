#ifndef __EVENT_TYPES_H__
#define __EVENT_TYPES_H__

#include <uv.h>
#include "stdtypes.h"
#include "cfg_tree.h"
#include "event_types.h"

typedef union {
    struct tim_ev {
        int tim_to;
        int tim_rep;
    } tim_ev;
    char *fs_path;
} handle_cfg_t;


typedef struct {
    uv_handle_t *handle;
    handle_cfg_t hcfg;
    void (*tim_cb)(uv_timer_t *handle);
    void (*fs_cb)(uv_fs_event_t *handle, const char *filename, int events, int status);
} ev_t;

typedef struct ev_glob {
    u8 id;
    u8 index;
    ev_t *ev;
    node_t *node;
    u8 measured_dev[32];
    u8 measured_dev_no;
} ev_glob_t;

#endif /* __EVENT_TYPES_H__ */