/**********************************************************************************************************************
* iio_buffer_types.h                                                                                                  *
***********************************************************************************************************************
* DESCRIPTION :                                                                                                       *
*       Types definition used when working with iio buffer in userspace.                                              *
*                                                                                                                     *
* PUBLIC TYPEDEFS :                                                                                                   *
*       iiobuff_format_t    stores data about the data format within the buffer.                                      *
*                                                                                                                     *
* PUBLIC FUNCTIONS :                                                                                                  *
*       ---                                                                                                           *
*                                                                                                                     *
* AUTHOR :                                                                                                            *
*       Yahor Yauseyenka    email: yahoryauseyenka@gmail.com                                                          *
*                                                                                                                     *
* VERSION                                                                                                             *
*       0.0.1               last modification: 10-04-2024                                                             *
*                                                                                                                     *
* LICENSE                                                                                                             *
*       GPL                                                                                                           *
*                                                                                                                     *
***********************************************************************************************************************/

#ifndef __IIO_BUFF_TYPES_H__
#define __IIO_BUFF_TYPES_H__

/**********************************************************************************************************************
 *  INCLUDES
 *********************************************************************************************************************/
#include <stdbool.h>
#include "stdtypes.h"

/**********************************************************************************************************************
 *  PREPROCESSOR DEFINITIONS
 *********************************************************************************************************************/


/**********************************************************************************************************************
 *  TYPEDEFS DEFINITION
 *********************************************************************************************************************/
typedef struct {
    u8 storagebits;
    bool ts_en;
} iiobuff_format_t;

/**********************************************************************************************************************
 * GLOBAL FUNCTION DECLARATION
 *********************************************************************************************************************/

#endif