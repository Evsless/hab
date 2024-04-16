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
 * GLOBAL VARIABLES DECLARATION
 *********************************************************************************************************************/
const u8  dev_idx_list[]  = HABDEV_IDX_SET;
const u32 trig_val_list[] = TRIG_PERIOD_SET;
const s16 trig_lut[] = TRIG_LUT;

uv_loop_t *loop;
uv_timer_t mprls_tim;

/**********************************************************************************************************************
 * LOCAL FUNCTION DECLARATION
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * LOCAL FUNCTION DEFINITION
 *********************************************************************************************************************/
void my_read(uv_timer_t *handle) {
    habdev_t *dev = (habdev_t *)handle->data;
    printf("Timer triggered %s\n", dev->dev_path);
    iiobuff_log2file(dev);
}

int mprls_task(int index) {
    loop = uv_default_loop();

    habdev_t *dev = habdev_get(index);
    mprls_tim.data = dev;

    uv_timer_init(loop, &mprls_tim);
    uv_timer_start(&mprls_tim, my_read, 0, 4000);

    return uv_run(loop, UV_RUN_DEFAULT);
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

    /* 2. DEVICE ALLOCATION */
    for (int i = 0; i < ARRAY_SIZE(dev_idx_list); i++) {
        habdev = habdev_alloc();
        habdev->trig = habtrig_get(trig_lut[i]);
        ret = habdev_register(habdev, dev_idx_list[i]);
        ret = iiobuff_setup(habdev);
    }
}

void hab_run(void) {
    int ret = 0;

    ret = mprls_task(IIO_KMOD_IDX_MPRLS0025);

}
