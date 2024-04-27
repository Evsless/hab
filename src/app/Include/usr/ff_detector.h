#ifndef __FREE_FALL_DETECTOR__
#define __FREE_FALL_DETECTOR__

#include "hab_device_types.h"

void ffdet_process_frame(const habdev_t *accel_dev);

#endif /* __FREE_FALL_DETECTOR__ */