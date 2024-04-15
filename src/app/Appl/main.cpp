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

    habtrig_t *habtrig = habtrig_alloc();
    ret = habtrig_register(habtrig, 500);

    habdev_t *mprls_dev    = habdev_alloc();
    mprls_dev->trig        = habtrig;
    mprls_dev->buff_format = mprls_iiobuff;

    ret = habdev_register(mprls_dev, IIO_KMOD_IDX_MPRLS0025);
    ret = iiobuff_setup(mprls_dev);

    
    // ret = iiobuff_log2file(iio_buffer, mprls_dev);
    
    habtrig_free(habtrig);
    habdev_free(mprls_dev);
    return 0;
}