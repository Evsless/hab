#include <stdio.h>

#include "stdtypes.h"
#include "hab_device.h"
#include "iio_buffer_ops.h"

#include "hab_trig.h"
#include "utils.h"


static const iiobuff_format_t mprls_iiobuff = {
    .storagebits = sizeof(s32),
    .ts_en = true,
};


u8  idx_arr[] = HABDEV_IDX_SET;
u32 trig_arr[] = TRIG_PERIOD_SET;
s16 trig_lut[] = TRIG_LUT;

int main(void) {
    stdret_t ret = STD_NOT_OK;

    habtrig_t *habtrig = NULL;
    habdev_t *habdev = NULL;

    /* 1. TRIGGER SETUP */
    for (int i = 0; i < ARRAY_SIZE(trig_arr); i++) {
        habtrig = habtrig_alloc();
        habtrig->index = i;
        if (trig_arr[i] > 0)
            ret = habtrig_register(habtrig, trig_arr[i]);
    }

    /* 2. DEVICE ALLOCATION */
    for (int i = 0; i < ARRAY_SIZE(idx_arr); i++) {
        habdev = habdev_alloc();
        habdev->trig = habtrig_get(trig_lut[i]);
        ret = habdev_register(habdev, idx_arr[i]);
        ret = iiobuff_setup(habdev);
    }

    return 0;
}