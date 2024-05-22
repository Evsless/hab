/***********************************************************************************************************************
 * event.cpp                                                                                                           *
 ***********************************************************************************************************************
 * DESCRIPTION :                                                                                                       *
 *       APIs for controlling application events (i.e. timer, filesystem, socket connection etc.)                      *
 * PUBLIC FUNCTIONS :                                                                                                  *
 *       ev_t*               event_alloc(void)                                                                         *
 *                                                                                                                     *
 * AUTHOR :                                                                                                            *
 *       Yahor Yauseyenka    email: yahoryauseyenka@gmail.com                                                          *
 *                                                                                                                     *
 * VERSION                                                                                                             *
 *       0.0.2               last modification: 20-04-2024                                                             *
 *                                                                                                                     *
 * LICENSE                                                                                                             *
 *       GPL                                                                                                           *
 *                                                                                                                     *
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include <stdlib.h>
#include <string.h>

#include "event.h"
#include "utils.h"
#include "hab_device_types.h"

#include "callback.h"

/***********************************************************************************************************************
 *  PREPROCESSOR DEFINITIONS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * LOCAL TYPEDEFS DECLARATION
 **********************************************************************************************************************/
typedef struct {
    int dev_idx;
    int cb_idx;
} dev_cb_ht_t;

/***********************************************************************************************************************
 * GLOBAL VARIABLES DECLARATION
 **********************************************************************************************************************/
static ev_glob_t *ev_glob_list[64];
static usize ev_glob_count = 0;

const u8 ev_tim_device[] = EV_TIM_DEV_IDX;
const char *ev_glob_name_list[] = EV_GLOB_LIST;
extern CALLBACK (*ev_global_tim_cb[64])(uv_timer_t *handle);

static dev_cb_ht_t dev_cb_ht[16];
static usize dev_cb_cnt;

/**********************************************************************************************************************
 * LOCAL FUNCTION DECLARATION
 *********************************************************************************************************************/
static stdret_t save_config(ev_glob_t *ev_glob, node_t *node, int cfg);

/***********************************************************************************************************************
 * LOCAL FUNCTION DEFINITION
 **********************************************************************************************************************/
static stdret_t save_config(ev_glob_t *ev_glob, node_t *node, int cfg) {
    stdret_t retval = STD_OK;
    int child_cnt = 0;

    cfg |= cfgtree_getCfgReg(node->type);

    switch (cfg & 0xFFFF) {
    case CFGTREE_EVENT:
        uv_handle_set_data(ev_glob->ev->handle, ev_glob);
        ev_glob->ev->tim_cb = ev_global_tim_cb[ev_glob->id];
        break;
    case CFGTREE_EVENT_TIM_TO_CONFIG:
        ev_glob->ev->hcfg.tim_ev.tim_to = atoi(node->val);
        break;
    case CFGTREE_EVENT_TIM_REP_CONFIG:
        ev_glob->ev->hcfg.tim_ev.tim_rep = atoi(node->val);
        break;
    case CFGTREE_INDEX:
        ev_glob->index = atoi(node->val);
        break;
    default:
        break;
    }

    if (STD_NOT_OK == retval)
        return STD_NOT_OK;

    while(child_cnt < node->child_num)
        save_config(ev_glob, node->child[child_cnt++], cfg);

    return retval;
}

/***********************************************************************************************************************
 * GLOBAL FUNCTION DEFINITION
 **********************************************************************************************************************/
void event_init(void) {
    for (usize i = 0; i < ARRAY_SIZE(ev_tim_device); i++) {
        dev_cb_ht[i].dev_idx = ev_tim_device[i];
        dev_cb_ht[i].cb_idx = i;
        dev_cb_cnt++;
    }
}

ev_t *event_alloc(void) {
    ev_t *event = NULL;
    uv_handle_t *handle = NULL;

    event = (ev_t *)malloc(sizeof(ev_t));
    if (NULL == event) {
        fprintf(stderr, "ERROR: Error when allocating event.\n");
        return NULL;
    }
    memset(event, 0, sizeof(ev_t));

    handle = (uv_handle_t *)malloc(sizeof(uv_handle_t) + sizeof(habdev_t));
    if (NULL == handle) {
        fprintf(stderr, "ERROR: Error when allocating event handle.\n");
        return NULL;
    }

    event->handle = handle;

    return event;
}

ev_glob_t *event_allocGlobalEv(void) {
    ev_glob_t *ev_glob = NULL;
    ev_t *ev = NULL;

    ev_glob = (ev_glob_t *)malloc(sizeof(ev_glob_t));
    if (NULL == ev_glob) {
        fprintf(stderr, "ERROR: Error during global event allocation.\n");
        return NULL;
    }
    memset(ev_glob, 0, sizeof(ev_glob_t));

    ev_glob_list[ev_glob_count++] = ev_glob;

    /* Should it be moved to config saving stage? */
    ev = event_alloc();
    if (NULL == ev)
        return NULL;
    ev_glob->ev = ev;

    return ev_glob;
}

stdret_t event_registerGlobalEv(ev_glob_t *ev_glob, const u8 index) {
    stdret_t retval = STD_NOT_OK;
    char path_buff[64]  = {0};
    char line[128]      = {0};
    usize file_pos      = 0;
    int readline_stat   = 0;

    ev_glob->id = index;

    xml_line_t *xml_cfg = llist_init();
    snprintf(path_buff, sizeof(path_buff), "%s%s", HAB_G_EV_CFG_PATH, ev_glob_name_list[index]);
    for (int i = 0; (readline_stat = get_line(path_buff, &file_pos, line, sizeof(line))) >= 0; i++)
        llist_push(xml_cfg, line);

    if (readline_stat == -2)
        return STD_NOT_OK;

    cfgtree_dfaReset();
    ev_glob->node = cfgtree_init(&xml_cfg);

    retval = save_config(ev_glob, ev_glob->node, 0);
    if (STD_NOT_OK == retval) {
        fprintf(stderr, "ERROR: Error writing configuration for global event: %s\n", ev_glob_name_list[ev_glob->id]);
        return STD_NOT_OK;
    }

    llist_free(xml_cfg);
    return STD_OK;
}

ev_glob_t *event_getGlobalEv(const int id) {
    ev_glob_t *retval = NULL;

    for (int i = 0; i < ev_glob_count; i++) {
        if (id == ev_glob_list[i]->id) {
            retval = ev_glob_list[i];
            break;
        }
    }

    return retval;
}

stdret_t event_addMeasuredDev(const int ev_glob_id, const int habdev_id) {
    ev_glob_t *ev_glob = NULL;

    ev_glob = event_getGlobalEv(ev_glob_id);
    if (NULL == ev_glob) {
        fprintf(stderr, "ERROR: Global event with %d id doesn't exist.\n", ev_glob_id);
        return STD_NOT_OK;
    }

    ev_glob->measured_dev[ev_glob->measured_dev_no++] = habdev_id;

    return STD_OK;
}

int event_getEvIdx(const int dev_idx) {
    int retval = -1;

    for (usize i = 0; i < dev_cb_cnt; i++) {
        if (dev_idx == dev_cb_ht[i].dev_idx) {
            retval = dev_cb_ht[i].cb_idx;
            break;
        }
    }

    return retval;
}

usize event_getDevIdx(const int idx) {
    return dev_cb_ht[idx].dev_idx;
}

usize event_getDevNum(void) {
    return dev_cb_cnt;
}

usize event_getGlobalNum(void) {
    return ARRAY_SIZE(ev_glob_name_list);
}

/***********************************************************************************************************************
 * END OF FILE
 **********************************************************************************************************************/
