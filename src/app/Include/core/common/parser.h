/**********************************************************************************************************************
* parser.h                                                                                                            *
***********************************************************************************************************************
* DESCRIPTION :                                                                                                       *
*       Header file with parser API && types declaration                                                              *
*                                                                                                                     *
* PUBLIC TYPEDEFS :                                                                                                   *
*       enum cfg_type_t          Configuration type (first element of config file stored in /cfg)                     *
*       enum cfg_field_t         Config field for a given type (second element of config file stored in /cfg)         *
*       enum cfg_t               Sum of configuration type and field                                                  *
*                                (both first and second element from file in /cfg)                                    *
*       struct cfgtoken_t        A configuration token that is received after parsing single line                     *
*                                                                                                                     *
* PUBLIC FUNCTIONS :                                                                                                  *
*       stdret_t            iiobuff_log2file(char *ubuff, const habdev_t *habdev);                                    *
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

#ifndef __PARSER_H__
#define __PARSER_H__

/**********************************************************************************************************************
 *  INCLUDES
 *********************************************************************************************************************/
#include "stdtypes.h"


/**********************************************************************************************************************
 *  PREPROCESSOR DEFINITIONS
 *********************************************************************************************************************/

/**********************************************************************************************************************
 *  TYPEDEF ENUM DECLARATION
 *********************************************************************************************************************/
typedef enum {
    CFG_EV_TIM    = 0,
    CFG_EV_FS     = 0x01 << 1,
    CFG_DEV_TYPE  = 0x01 << 2,
    CFG_BUFF      = 0x01 << 3,
    CFG_CMD       = 0x01 << 4,
    CFG_MEAS_CHAN = 0x01 << 7,
} cfg_type_t;

typedef enum {
    /* TIMER EVENT CONFIGURATION FIELDS */
    CFGF_TIMEOUT = 0,
    CFGF_REPEAT  = 1,
    /* FS EVENT CONFIG FIELDS */
    CFGF_FILEPATH = 0,
    /* DEVICE TYPE CONFIGURATION FIELDS */
    CFGF_IIO = 0,
    CFGF_IIO_BUFF = 1,
    CFGF_DEFAULT = 2,
    CFGF_CAMERA = 3,
    /* BUFFER CONFIGURATION FIELDS */
    CFGF_BUFF_LEN = 0,
    CFGF_BUFF_EN  = 1,
    /* COMMANDS CONFIGURATION FIELDS */
    CFGF_CAM_STILL = 0,
    CFGF_CAM_VID = 1,
    /* MEASURED CHANNELS CONFIGURATION FIELDS */
    CFGF_CHAN_IN_TS    = 0,
    CFGF_CHAN_IN_V0_V1 = 1,
    CFGF_CHAN_IN_V2_V3 = 2,
    CFGF_CHAN_IN_TEMP_AMBIENT = 3,
    CFGF_CHAN_IN_TEMP_OBJECT  = 4,
    CFGF_CHAN_OUT_RESISTANCE = 5,
    CFGF_CHAN_IN_ACCEL_X = 6,
    CFGF_CHAN_IN_ACCEL_Y = 7,
    CFGF_CHAN_IN_ACCEL_Z = 8,
    CFGF_CHAN_CUSTOM = 255,
} cfg_field_t;

typedef enum {
    CFG_EV_TIMEOUT   = CFG_EV_TIM | CFGF_TIMEOUT,    /* 0x00 */
    CFG_EV_REPEAT    = CFG_EV_TIM | CFGF_REPEAT,     /* 0x01 */
    CFG_EV_FILEPATH  = CFG_EV_FS | CFGF_FILEPATH,    /* 0x02 */
    CFG_DEV_IIO      = CFG_DEV_TYPE | CFGF_IIO,      /* 0x04 */
    CFG_DEV_IIO_BUFF = CFG_DEV_TYPE | CFGF_IIO_BUFF, /* 0x05 */
    CFG_DEV_NON_IIO  = CFG_DEV_TYPE | CFGF_DEFAULT,  /* 0x06 */
    CFG_DEV_CAMERA   = CFG_DEV_TYPE | CFGF_CAMERA,   /* 0x07 */
    CFG_BUFF_LEN     = CFG_BUFF | CFGF_BUFF_LEN,     /* 0x08 */
    CFG_BUFF_EN      = CFG_BUFF | CFGF_BUFF_EN,      /* 0x09 */
    CFG_CAM_STILL    = CFG_CMD  | CFGF_CAM_STILL,    /* 0x16 */
    CFG_CAM_VID      = CFG_CMD  | CFGF_CAM_VID,      /* 0x17 */
    CFG_CH_IN_TS            = CFG_MEAS_CHAN | CFGF_CHAN_IN_TS,           /* 0x80 */
    CFG_CH_IN_V0_V1         = CFG_MEAS_CHAN | CFGF_CHAN_IN_V0_V1,        /* 0x81 */
    CFG_CH_IN_V2_V3         = CFG_MEAS_CHAN | CFGF_CHAN_IN_V2_V3,        /* 0x82 */
    CFG_CH_IN_TEMP_AMBIENT  = CFG_MEAS_CHAN | CFGF_CHAN_IN_TEMP_AMBIENT, /* 0x83 */
    CFG_CH_IN_TEMP_OBJECT   = CFG_MEAS_CHAN | CFGF_CHAN_IN_TEMP_OBJECT,  /* 0x84 */
    CFG_CH_OUT_RESISTANCE   = CFG_MEAS_CHAN | CFGF_CHAN_OUT_RESISTANCE,  /* 0x85 */
    CFG_CH_ACCEL_X          = CFG_MEAS_CHAN | CFGF_CHAN_IN_ACCEL_X,      /* 0x86 */
    CFG_CH_ACCEL_Y          = CFG_MEAS_CHAN | CFGF_CHAN_IN_ACCEL_Y,      /* 0x87 */
    CFG_CH_ACCEL_Z          = CFG_MEAS_CHAN | CFGF_CHAN_IN_ACCEL_Z,      /* 0x88 */
    CFG_CH_CUSTOM           = CFG_MEAS_CHAN | CFGF_CHAN_CUSTOM,          /* 0xFF */
} cfg_t;

/**********************************************************************************************************************
 *  TYPEDEF STRUCT DECLARATION
 *********************************************************************************************************************/
typedef struct {
    cfg_t cfg_type;
    char val[256];
} cfgtoken_t;


/**********************************************************************************************************************
 * GLOBAL FUNCTION DECLARATION
 *********************************************************************************************************************/

cfgtoken_t parser_parse(const char *cfg);
const char *parser_get_chan(const cfg_t config);

#endif /* __PARSER_H__ */

/***********************************************************************************************************************
 * END OF FILE
 **********************************************************************************************************************/
 