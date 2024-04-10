#include <stdio.h>

#include "stdtypes.h"
#include "hab_device.h"
#include "iio_buffer_ops.h"

#define IIO_BUFF_SIZE       1024
#define MPRLS0025_BUFF_PATH "/dev/iio:device0"
#define MPRLS0025_DEV_PATH  "/sys/bus/iio/devices/iio:device0/"
#define MPRLS0025_LOG_PATH  "/media/hab_flight_data/mprls0025.log"

#define BUFF_DATA_AVAIL "buffer/data_available"

char iio_buffer[IIO_BUFF_SIZE];

static const iiobuff_format_t mprls_iiobuff = {
    .storagebits = sizeof(s32),
    .ts_en = true,
};

int main(void) {
    stdret_t ret = STD_NOT_OK;

    habdev_t *mprls_dev    = habdev_init();
    mprls_dev->dev_path    = MPRLS0025_DEV_PATH;
    mprls_dev->buff_path   = MPRLS0025_BUFF_PATH;
    mprls_dev->log_path    = MPRLS0025_LOG_PATH;
    mprls_dev->buff_format = mprls_iiobuff;

    ret = iiobuff_log2file(iio_buffer, mprls_dev);
    
    habdev_free(mprls_dev);
    return 0;
}