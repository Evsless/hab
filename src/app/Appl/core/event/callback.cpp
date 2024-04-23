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
*       CALLBACK            ICM20X_CALLBACK(uv_timer_t *handle)                                                       *
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
#include "iio_buffer_ops.h"

#include "wheatstone.h"

/**********************************************************************************************************************
 *  PREPROCESSOR DEFINITIONS
 *********************************************************************************************************************/


/**********************************************************************************************************************
 * LOCAL TYPEDEFS DECLARATION
 *********************************************************************************************************************/


/**********************************************************************************************************************
 * GLOBAL VARIABLES DECLARATION
 *********************************************************************************************************************/

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
    habdev_t *ads1115_48 = (habdev_t *)uv_handle_get_data((uv_handle_t *)handle);
    wheatstone_run(ads1115_48);
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

/***********************************************************************************************************************
 * END OF FILE
 **********************************************************************************************************************/
