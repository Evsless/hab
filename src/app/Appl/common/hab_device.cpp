#include <stdlib.h>
#include "hab_device.h"

habdev_t *habdev_init(void) {
    habdev_t *habdev = NULL;

    habdev = (habdev_t *)malloc(sizeof(habdev_t));
    return habdev;
}

void habdev_free(habdev_t *dev) {
    free(dev);
}