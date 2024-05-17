/**********************************************************************************************************************
* callback.cpp                                                                                                        *
***********************************************************************************************************************
* DESCRIPTION :                                                                                                       *
*       Callback definition for every device that has event attached. All the code should                             *
*       be preferrably written in Appls/usr folder. The platform code will only call it.                              *
*                                                                                                                     *
* PUBLIC FUNCTIONS :                                                                                                  *
*       CALLBACK            ADS1115_48_CALLBACK(uv_timer_t *handle)                                                   *
*       CALLBACK            ADS1115_49_CALLBACK(uv_timer_t *handle)                                                   *
*       CALLBACK            MPRLS0025_CALLBACK(uv_timer_t *handle)                                                    *
*       CALLBACK            MLX90614_CALLBACK(uv_timer_t *handle)                                                     *
*       CALLBACK            ICM20948_CALLBACK(uv_timer_t *handle)                                                       *
*       CALLBACK            SHT4X_CALLBACK(uv_timer_t *handle)                                                        *
*                                                                                                                     *
* AUTHOR :                                                                                                            *
*       Yahor Yauseyenka    email: yahoryauseyenka@gmail.com                                                          *
*                                                                                                                     *
* VERSION                                                                                                             *
*       0.0.1               last modification: 23-04-2024                                                             *
*                                                                                                                     *
* LICENSE                                                                                                             *
*       GPL                                                                                                           *
*                                                                                                                     *
***********************************************************************************************************************/

/**********************************************************************************************************************
 *  INCLUDES
 *********************************************************************************************************************/
#include <string.h>

#include "utils.h"
#include "callback.h"
#include "hab_device.h"

#include "camera.h"
#include "task_main.h"
#include "wheatstone.h"
#include "ff_detector.h"

#include "iio_buffer_ops.h"
/**********************************************************************************************************************
 *  PREPROCESSOR DEFINITIONS
 *********************************************************************************************************************/


/**********************************************************************************************************************
 * LOCAL TYPEDEFS DECLARATION
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * GLOBAL VARIABLES DECLARATION
 *********************************************************************************************************************/
CALLBACK (*ev_tim_callback_list[64])(uv_timer_t *handle) = HAB_CALLBACKS;
CALLBACK (*ev_global_tim_cb[64])(uv_timer_t *handle) = EV_GLOBAL_CB_LIST;

/**********************************************************************************************************************
 * LOCAL FUNCTION DECLARATION
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * LOCAL FUNCTION DEFINITION
 *********************************************************************************************************************/


/**********************************************************************************************************************
 * GLOBAL FUNCTION DEFINITION
 *********************************************************************************************************************/
#ifdef MPRLS0025_CALLBACK
CALLBACK MPRLS0025_CALLBACK(uv_timer_t *handle) {
    habdev_t *mprls_dev = (habdev_t *)uv_handle_get_data((uv_handle_t *)handle);
    (void)iiobuff_log2file(mprls_dev, NULL, NULL);
}
#endif

#ifdef ICM20948_CALLBACK
CALLBACK ICM20948_CALLBACK(uv_timer_t *handle) {
    habdev_t *icm20x_dev = (habdev_t *)uv_handle_get_data((uv_handle_t *)handle);
    printf("%s\n", icm20x_dev->path.dev_name);
    // ffdet_process_frame(icm20x_dev);

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
    habdev_t *ads1115_48 = (habdev_t *)uv_handle_get_data((uv_handle_t *)handle);
    wheatstone_run(ads1115_48);
}
#endif

#ifdef ADS1115_49_CALLBACK
CALLBACK ADS1115_49_CALLBACK(uv_timer_t *handle) {
    habdev_t *ads1115_49 = (habdev_t *)uv_handle_get_data((uv_handle_t *)handle);
    wheatstone_run(ads1115_49);
}
#endif

#ifdef MLX90614_CALLBACK
CALLBACK MLX90614_CALLBACK(uv_timer_t *handle) {
    habdev_t *habdev = (habdev_t *)uv_handle_get_data((uv_handle_t *)handle);
    printf("%s\n", habdev->path.dev_name);
}
#endif

#ifdef IMX477_01_CALLBACK
CALLBACK IMX477_01_CALLBACK(uv_timer_t *handle) {
    habdev_t *habcam_1 = (habdev_t *)uv_handle_get_data((uv_handle_t *)handle);
    camera_run(habcam_1);
}
#endif

#ifdef EV_MAIN_CALLBACK
CALLBACK EV_MAIN_CALLBACK(uv_timer_t *handle) {
    ev_glob_t *ev_main = (ev_glob_t *)uv_handle_get_data((uv_handle_t *)handle);
    task_runMain(ev_main);
}
#endif
/***********************************************************************************************************************
 * END OF FILE
 **********************************************************************************************************************/
