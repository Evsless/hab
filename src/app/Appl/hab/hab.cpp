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
#include "hab_trig.h"
#include "hab_device.h"
#include "utils.h"
#include "iio_buffer_ops.h"

/**********************************************************************************************************************
 *  PREPROCESSOR DEFINITIONS
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * LOCAL FUNCTION DECLARATION
 *********************************************************************************************************************/
CALLBACK MPRLS0025_CALLBACK(uv_timer_t *handle);
CALLBACK SHT4X_CALLBACK(uv_timer_t *handle);

// CALLBACK ICM20X_CALLBACK(uv_timer_t *handle);

/**********************************************************************************************************************
 * GLOBAL VARIABLES DECLARATION
 *********************************************************************************************************************/
uv_loop_t *loop;

const u8  dev_idx_list[]  = HABDEV_IDX_SET;
const u32 trig_val_list[] = TRIG_PERIOD_SET;
const s16 trig_lut[] = TRIG_LUT;

const u8 ev_tim_device[] = DEV_TIM_DEV_IDX;

CALLBACK (*ev_tim_callback_list[])(uv_timer_t *handle) = {
    MPRLS0025_CALLBACK,
    SHT4X_CALLBACK,
};


/**********************************************************************************************************************
 * LOCAL FUNCTION DEFINITION
 *********************************************************************************************************************/
CALLBACK MPRLS0025_CALLBACK(uv_timer_t *handle) {
    habdev_t *habdev = (habdev_t *)uv_handle_get_data((uv_handle_t *)handle);
    printf("%s\n", habdev->path.dev_path);
}

CALLBACK SHT4X_CALLBACK(uv_timer_t *handle) {
    habdev_t *habdev = (habdev_t *)uv_handle_get_data((uv_handle_t *)handle);
    printf("%s\n", habdev->path.dev_path);
}


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

    /* 1. TRIGGER SETUP */
    for (int i = 0; i < ARRAY_SIZE(trig_val_list); i++) {
        habtrig = habtrig_alloc();
        habtrig->index = i;
        if (trig_val_list[i] > 0)
            ret = habtrig_register(habtrig, trig_val_list[i]);
    }

    // /* 2. DEVICE ALLOCATION */
    for (int i = 0; i < ARRAY_SIZE(dev_idx_list); i++) {
        habdev = habdev_alloc();
        habdev->trig = habtrig_get(trig_lut[i]);
        ret = habdev_register(habdev, dev_idx_list[i]);
        ret = iiobuff_setup(habdev);
        ret = ev_setup(habdev->event, habdev->path.dev_name);
    }

    /* 3. Assign timer callbacks */
    for (int i = 0; i < ARRAY_SIZE(ev_tim_device); i++) {
        habdev = habdev_get(ev_tim_device[i]);
        habdev->event->tim_cb = ev_tim_callback_list[i];
    }
}

int hab_run(void) {
    int ret = 0;
    habdev_t *habdev = NULL;

    /* SINGLE THREAD VERSION */
    loop = uv_default_loop();
    
    habdev = habdev_get(0);
    habdev = habdev_get(1);
    for (int i = 0; i < ARRAY_SIZE(ev_tim_device); i++) {
        habdev = habdev_get(i);
        run_tim_ev(habdev);
    }

    return uv_run(loop, UV_RUN_DEFAULT);
}
