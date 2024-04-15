#include <stdio.h>

#include "stdtypes.h"
#include "hab_device.h"
#include "iio_buffer_ops.h"

#include "hab_trig.h"

#define IIO_BUFF_SIZE       1024

#define BUFF_DATA_AVAIL "buffer/data_available"


char iio_buffer[IIO_BUFF_SIZE];

static const iiobuff_format_t mprls_iiobuff = {
    .storagebits = sizeof(s32),
    .ts_en = true,
};

int main(void) {
    stdret_t ret = STD_NOT_OK;

    /* HRTRIG SETUP */
    habtrig_t *habtrig = habtrig_alloc();
    ret = habtrig_register(habtrig, 500);

    /* IRQ TRIG SETUP */
    habtrig_t *icm20x_trig = habtrig_alloc();

    /* MPRLS DEVICE ALLOC */
    habdev_t *mprls_dev    = habdev_alloc();
    mprls_dev->trig        = habtrig;
    mprls_dev->buff_format = mprls_iiobuff;

    ret = habdev_register(mprls_dev, IIO_KMOD_IDX_MPRLS0025);
    ret = iiobuff_setup(mprls_dev);

    /* ICM */
    habdev_t *icm20x_dev = habdev_alloc();
    icm20x_dev->trig = icm20x_trig;

    ret = habdev_register(icm20x_dev, IIO_KMOD_IDX_ICM20X);
    ret = iiobuff_setup(icm20x_dev);

    
    // ret = iiobuff_log2file(iio_buffer, mprls_dev);
    
    // habtrig_free(habtrig);
    // habdev_free(mprls_dev);
    return 0;
}