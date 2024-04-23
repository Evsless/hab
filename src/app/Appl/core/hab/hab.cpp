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

/**********************************************************************************************************************
 *  PREPROCESSOR DEFINITIONS
 *********************************************************************************************************************/


/**********************************************************************************************************************
 * GLOBAL VARIABLES DECLARATION
 *********************************************************************************************************************/
const u8  dev_idx_list[]  = HABDEV_IDX_SET;
const u32 trig_val_list[] = TRIG_PERIOD_SET;
const s8 trig_lut[]       = TRIG_LUT;

const u8 ev_tim_device[] = EV_TIM_DEV_IDX;

uv_loop_t *loop;

CALLBACK (*ev_tim_callback_list[])(uv_timer_t *handle) = HAB_CALLBACKS;

/**********************************************************************************************************************
 * LOCAL FUNCTION DECLARATION
 *********************************************************************************************************************/


/**********************************************************************************************************************
 * LOCAL FUNCTION DEFINITION
 *********************************************************************************************************************/
void run_tim_ev(habdev_t *habdev) {
    int timeout = habdev->event->hcfg.tim_ev.tim_to;
    int repeat = habdev->event->hcfg.tim_ev.tim_rep;

    uv_timer_init(loop, (uv_timer_t *)habdev->event->handle);
    uv_timer_start((uv_timer_t *)habdev->event->handle, habdev->event->tim_cb, timeout, repeat);
}

/**********************************************************************************************************************
 * GLOBAL FUNCTION DEFINITION
 *********************************************************************************************************************/
void hab_init(void) {
    stdret_t ret = STD_NOT_OK;
    habtrig_t *habtrig = NULL;
    habdev_t *habdev = NULL;
    ev_t *event = NULL;

    /* 1. TRIGGER SETUP */
    for (int i = 0; i < ARRAY_SIZE(trig_val_list); i++) {
        habtrig = habtrig_alloc();
        habtrig->index = i;
        if (trig_val_list[i] > 0)
            ret = habtrig_register(habtrig, trig_val_list[i]);
    }

    /* 2. DEVICE ALLOCATION */
    for (int i = 0; i < ARRAY_SIZE(dev_idx_list); i++) {
        habdev = habdev_alloc();
        ret = habdev_register(habdev, dev_idx_list[i]);

        if (trig_lut[i] != -1) {
            habdev_trig_set(habdev, habtrig_get(trig_lut[i]));
            ret = iiobuff_setup(habdev);
        } 
        else {
            ret = habdev_iio_data_setup(habdev);
            /* Allocate device specific data for non-triggered devs */
        }
    }

    for (int i = 0; i < ARRAY_SIZE(ev_tim_device); i++) {
        habdev = habdev_get(ev_tim_device[i]);
    
        event = event_alloc();
        habdev_ev_set(habdev, event);
        ret = ev_setup(habdev);
    
        habdev->event->tim_cb = ev_tim_callback_list[i];
    }
}

int hab_run(void) {
    int ret = 0;
    habdev_t *habdev = NULL;

    loop = uv_default_loop();

    for (int i = 0; i < ARRAY_SIZE(ev_tim_device); i++) {
        habdev = habdev_get(ev_tim_device[i]);
        run_tim_ev(habdev);
    }

    return uv_run(loop, UV_RUN_DEFAULT);
}
