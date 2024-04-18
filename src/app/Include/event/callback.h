#ifndef __CALLBACK_H__
#define __CALLBACK_H__

#define CALLBACK void

#ifdef MPRLS0025_CALLBACK
CALLBACK MPRLS0025_CALLBACK(uv_timer_t *handle);
#endif

#ifdef SHT4X_CALLBACK
CALLBACK SHT4X_CALLBACK(uv_timer_t *handle);
#endif

#ifdef ICM20X_CALLBACK
CALLBACK ICM20X_CALLBACK(uv_timer_t *handle);
#endif

#ifdef ADS1115_48_CALLBACK
CALLBACK ADS1115_48_CALLBACK(uv_timer_t *handle);
#endif

#ifdef ADS1115_49_CALLBACK
CALLBACK ADS1115_49_CALLBACK(uv_timer_t *handle);
#endif

#ifdef MLX90614_CALLBACK
CALLBACK MLX90614_CALLBACK(uv_timer_t *handle);
#endif




#ifdef MPRLS0025_CALLBACK
CALLBACK MPRLS0025_CALLBACK(uv_timer_t *handle) {
    habdev_t *habdev = (habdev_t *)uv_handle_get_data((uv_handle_t *)handle);
    printf("%s\n", habdev->path.dev_name);
}
#endif

#ifdef ICM20X_CALLBACK
CALLBACK ICM20X_CALLBACK(uv_timer_t *handle) {
    habdev_t *habdev = (habdev_t *)uv_handle_get_data((uv_handle_t *)handle);
    printf("%s\n", habdev->path.dev_name);
}
#endif

#ifdef SHT4X_CALLBACK
CALLBACK SHT4X_CALLBACK(uv_timer_t *handle) {
    habdev_t *habdev = (habdev_t *)uv_handle_get_data((uv_handle_t *)handle);
    printf("%s\n", habdev->path.dev_name);
}
#endif

#ifdef ADS1115_48_CALLBACK
CALLBACK ADS1115_48_CALLBACK(uv_timer_t *handle) {
    habdev_t *habdev = (habdev_t *)uv_handle_get_data((uv_handle_t *)handle);
    printf("%s\n", habdev->path.dev_name);
}
#endif

#ifdef ADS1115_49_CALLBACK
CALLBACK ADS1115_49_CALLBACK(uv_timer_t *handle) {
    habdev_t *habdev = (habdev_t *)uv_handle_get_data((uv_handle_t *)handle);
    printf("%s\n", habdev->path.dev_name);
}
#endif

#ifdef MLX90614_CALLBACK
CALLBACK MLX90614_CALLBACK(uv_timer_t *handle) {
    habdev_t *habdev = (habdev_t *)uv_handle_get_data((uv_handle_t *)handle);
    printf("%s\n", habdev->path.dev_name);
}
#endif

#endif /* __CALLBACK_H__ */