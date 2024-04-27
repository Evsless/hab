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

#include "wheatstone.h"
#include "ff_detector.h"

#include <math.h>
#include <time.h>

// double
// epoch_double(struct timespec *tv)
// {
//   char time_str[32];

//   sprintf(time_str, "%ld.%.9ld", tv->tv_sec, tv->tv_nsec);

//   return atof(time_str);
// }

// long int
// epoch_millis(struct timespec *tv)
// {
//   double epoch;
//   epoch = epoch_double(tv);
//   epoch = round(epoch*1e3);

//   return (long int) epoch;
// }
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

long int prev = 0;

#ifdef ICM20X_CALLBACK
CALLBACK ICM20X_CALLBACK(uv_timer_t *handle) {
    // struct timespec tv;
    // clock_gettime(CLOCK_REALTIME, &tv);

    // long int ms = epoch_millis(&tv);
    // printf("TE: %lld\n", ms - prev);
    // prev = ms;

    habdev_t *icm20x_dev = (habdev_t *)uv_handle_get_data((uv_handle_t *)handle);
    ffdet_process_frame(icm20x_dev);

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
