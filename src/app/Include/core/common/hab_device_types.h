/**********************************************************************************************************************
* hab_device_types.h                                                                                                  *
***********************************************************************************************************************
* DESCRIPTION :                                                                                                       *
*       Header file that contains types definition for hab_device module                                              *
*                                                                                                                     *
* PUBLIC TYPEDEFS :                                                                                                   *
*       enum dev_type_t     Type of registered device. Used to handle R/W operation for a device                      *
*       struct hab_path_t   Set of fs paths that are commonly used when controllin I/O device                         *
*       struct data_format_t                                                                                          *
*                           Defines a format how data is represented (for instance in a buffer)                       *
*       struct hab_path_t   Set of fs paths that are commonly used when controllin I/O device                         *
*       struct habdev_t     Set of data related to connected device. Main platform structure                          *
*                                                                                                                     *
* PUBLIC FUNCTIONS :                                                                                                  *
*       ----                                                                                                          *
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

#ifndef __HAB_DEVICE_TYPES_H__
#define __HAB_DEVICE_TYPES_H__

/**********************************************************************************************************************
 *  INCLUDES
 *********************************************************************************************************************/
#include "hab_trig.h"
#include "cfg_tree.h"
#include "event_types.h"

/**********************************************************************************************************************
 *  PREPROCESSOR DEFINITIONS
 *********************************************************************************************************************/

/**********************************************************************************************************************
 *  TYPEDEF ENUM DECLARATION
 *********************************************************************************************************************/
typedef enum {
    DEV_IIO,
    DEV_IIO_BUFF,
    DEV_CAMERA,
    DEV_DEFAULT,
    DEV_UNKNOWN,
} dev_type_t;

/**********************************************************************************************************************
 *  TYPEDEF STRUCT DECLARATION
 *********************************************************************************************************************/
typedef struct {
    char dev_name[16];
    char *channel[16];
    char *buffer[4];
} hab_path_t;

typedef struct {
    u8 chan_num;
    u8 storagebits[16];
    bool ts_en;
} data_format_t;

typedef struct {
    u8 id;
    u8 index;
    ev_t *event;
    hab_path_t path;
    habtrig_t *trig;
    data_format_t df;
    u32 channel_num;
    u32 buffer_num;
    node_t *node;
    dev_type_t dev_type;
} habdev_t;

/**********************************************************************************************************************
 * GLOBAL FUNCTION DECLARATION
 *********************************************************************************************************************/

#endif /* __HAB_DEVICE_TYPES_H__ */

/***********************************************************************************************************************
 * END OF FILE
 **********************************************************************************************************************/
 