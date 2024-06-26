/**********************************************************************************************************************
* callback.h                                                                                                          *
***********************************************************************************************************************
* DESCRIPTION :                                                                                                       *
*       Header file that contais callbacks declaration                                                                *
*                                                                                                                     *
* PUBLIC TYPEDEFS :                                                                                                   *
*       ----                                                                                                          *
*                                                                                                                     *
* PUBLIC FUNCTIONS :                                                                                                  *
*       CALLBACK            ADS1115_48_CALLBACK(uv_timer_t *handle);                                                  *
*       CALLBACK            ADS1115_49_CALLBACK(uv_timer_t *handle);                                                  *
*       CALLBACK            ADS1115_49_CALLBACK(uv_timer_t *handle);                                                  *
*       CALLBACK            MLX90614_CALLBACK(uv_timer_t *handle);                                                    *
*       CALLBACK            ICM20948_CALLBACK(uv_timer_t *handle);                                                      *
*       CALLBACK            SHT4X_CALLBACK(uv_timer_t *handle);                                                       *
*                                                                                                                     *
* AUTHOR :                                                                                                            *
*       Yahor Yauseyenka    email: yahoryauseyenka@gmail.com                                                          *
*                                                                                                                     *
* VERSION                                                                                                             *
*       0.0.1               last modification: 20-04-2024                                                             *
*                                                                                                                     *
* LICENSE                                                                                                             *
*       GPL                                                                                                           *
*                                                                                                                     *
***********************************************************************************************************************/

#ifndef __CALLBACK_H__
#define __CALLBACK_H__

/**********************************************************************************************************************
 *  INCLUDES
 *********************************************************************************************************************/
#include <uv.h>
#include "ev_glob.h"
/**********************************************************************************************************************
 *  PREPROCESSOR DEFINITIONS
 *********************************************************************************************************************/
#define CALLBACK void

/**********************************************************************************************************************
 *  TYPEDEF ENUM DECLARATION
 *********************************************************************************************************************/

/**********************************************************************************************************************
 *  TYPEDEF STRUCT DECLARATION
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * GLOBAL FUNCTION DECLARATION
 *********************************************************************************************************************/
#ifdef MPRLS0025_CALLBACK
CALLBACK MPRLS0025_CALLBACK(uv_timer_t *handle);
#endif

#ifdef SHT4X_CALLBACK
CALLBACK SHT4X_CALLBACK(uv_timer_t *handle);
#endif

#ifdef ICM20948_CALLBACK
CALLBACK ICM20948_CALLBACK(uv_timer_t *handle);
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

#ifdef IMX477_01_CALLBACK
CALLBACK IMX477_01_CALLBACK(uv_timer_t *handle);
#endif

#ifdef IMX477_02_CALLBACK
CALLBACK IMX477_02_CALLBACK(uv_timer_t *handle);
#endif

#ifdef EV_MAIN_CALLBACK
CALLBACK EV_MAIN_CALLBACK(uv_timer_t *handle);
#endif

#endif /* __CALLBACK_H__ */

/***********************************************************************************************************************
 * END OF FILE
 **********************************************************************************************************************/
 