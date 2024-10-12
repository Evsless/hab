/**********************************************************************************************************************
* hab.cpp                                                                                                        *
***********************************************************************************************************************
* DESCRIPTION :                                                                                                       *
*       Module for creating and registering triggers, that later will be used                                         *
*       for triggering iio buffer.                                                                                    *
*                                                                                                                     *
* PUBLIC FUNCTIONS :                                                                                                  *
*       habtrig_t*          habtrig_alloc(void);                                                                      *
*       void                habtrig_free(habtrig_t *trig);                                                            *
*       stdret_t            habtrig_register(habtrig_t *trig, u32 period_ms);                                         *
*                                                                                                                     *
* AUTHOR :                                                                                                            *
*       Yahor Yauseyenka    email: yahoryauseyenka@gmail.com                                                          *
*                                                                                                                     *
* VERSION                                                                                                             *
*       0.0.1               last modification: 16-04-2024                                                             *
*                                                                                                                     *
* LICENSE                                                                                                             *
*       GPL                                                                                                           *
*                                                                                                                     *
***********************************************************************************************************************/

/**********************************************************************************************************************
 *  INCLUDES
 *********************************************************************************************************************/
#include <uv.h>

#include "hab.h"
#include "utils.h"
#include "event.h"
#include "callback.h"
#include "hab_trig.h"
#include "hab_device.h"
#include "iio_buffer_ops.h"

/* UGLY QUICK FIX. REWORK */
#include <string.h>

/**********************************************************************************************************************
 *  PREPROCESSOR DEFINITIONS
 *********************************************************************************************************************/


/**********************************************************************************************************************
 * GLOBAL VARIABLES DECLARATION
 *********************************************************************************************************************/
const u8  dev_idx_list[]  = HABDEV_IDX_SET;
const u32 trig_val_list[] = TRIG_PERIOD_SET;

uv_loop_t *loop;
uv_work_t work;

/**********************************************************************************************************************
 * LOCAL FUNCTION DECLARATION
 *********************************************************************************************************************/


/**********************************************************************************************************************
 * LOCAL FUNCTION DEFINITION
 *********************************************************************************************************************/
void run_tim_ev(ev_t *event) {
    int timeout = event->hcfg.tim_ev.tim_to;
    int repeat = event->hcfg.tim_ev.tim_rep;

    uv_timer_init(loop, (uv_timer_t *)event->handle);
    uv_timer_start((uv_timer_t *)event->handle, event->tim_cb, timeout, repeat);
}

/**********************************************************************************************************************
 * GLOBAL FUNCTION DEFINITION
 *********************************************************************************************************************/
void hab_init(void) {
    stdret_t ret = STD_NOT_OK;
    habtrig_t *habtrig = NULL;
    habdev_t *habdev = NULL;
    ev_glob_t *event = NULL;

    char led_buff[4] = {0};

    /* 1. TRIGGER SETUP */
    for (int i = 0; i < ARRAY_SIZE(trig_val_list); i++) {
        habtrig = habtrig_alloc();
        habtrig->index = i;
        if (trig_val_list[i] > 0)
            ret = habtrig_register(habtrig, trig_val_list[i]);
    }

    habdev_preinit();
    /* 2.0 Global event allocation */
    for (int i = 0; i < event_getGlobalNum(); i++) {
        event = event_allocGlobalEv();
        event_registerGlobalEv(event, i);
    }

    /* 2. DEVICE ALLOCATION */
    for (int i = 0; i < ARRAY_SIZE(dev_idx_list); i++) {
        habdev = habdev_alloc();
        ret = habdev_register(habdev, dev_idx_list[i]);
    
        if (ret == STD_NOT_OK) {
            fprintf(stderr, "Error registering the device: %s\n", habdev->path.dev_name);
            /* Error loop instead of exiting? */
            // exit(-1);
        }
    }
    habdev_postinit();

    snprintf(led_buff, sizeof(ret), "%d", ret);
    write_file(HAB_LED_PATH, led_buff, sizeof(led_buff), MOD_W);
}

int hab_run(void) {
    int ret = 0;
    habdev_t *habdev = NULL;
    ev_glob_t *event = NULL;

    loop = uv_default_loop();

    for (int i = 0; i < event_getDevNum(); i++) {
        habdev = habdev_get(event_getDevIdx(i));
        if (NULL != habdev)
            run_tim_ev(habdev->event);
    }

    for (int i = 0; i < event_getGlobalNum(); i++) {
        event = event_getGlobalEv(i);
        if (NULL != event)
            run_tim_ev(event->ev);
    }

    return uv_run(loop, UV_RUN_DEFAULT);
}
