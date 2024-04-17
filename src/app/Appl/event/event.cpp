#include <stdlib.h>

#include "event.h"
#include "utils.h"
#include "parser.h"
#include "hab_device_types.h"

ev_t *event_alloc(void) {
    ev_t *event = NULL;
    uv_handle_t *handle = NULL;

    event = (ev_t *)malloc(sizeof(ev_t));
    handle = (uv_handle_t *)malloc(sizeof(uv_handle_t) + sizeof(habdev_t));

    event->handle = handle;

    return event;
}

stdret_t ev_setup(habdev_t *habdev) {
    stdret_t ret = STD_NOT_OK;
    ev_cfgtoken_t token;
    usize file_pos = 0;

    ev_t *event = habdev->event;
    const char *dev_name = habdev->path.dev_name;

    char filebuff[128] = {0};
    char tmp[64] = {0};
    
    /* Setup a path to event config file */
    ret = create_path(filebuff, 2, HAB_EV_CFG_PATH, dev_name);

    while (get_line(filebuff, &file_pos, tmp, sizeof(tmp)) >= 0) {
        token = parser_evcfg(tmp);

        if (CFG_EV_TIMEOUT == token.cfg_type) {
            event->hcfg.tim_ev.tim_to = atoi(token.val);
        } else if (token.cfg_type == CFG_EV_REPEAT) {
            event->hcfg.tim_ev.tim_rep = atoi(token.val);
        }
    }
    
    return ret;
}