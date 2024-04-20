#ifndef __PARSER_H__
#define __PARSER_H__

#include "stdtypes.h"

typedef enum {
    CFG_EV_TIM    = 0,
    CFG_EV_FS     = 0x01 << 1,
    CFG_DEV_TYPE  = 0x01 << 2,
    CFG_BUFF      = 0x01 << 3,
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
    /* BUFFER CONFIGURATION FIELDS */
    CFGF_BUFF_LEN = 0,
    CFGF_BUFF_EN  = 1,
    /* MEASURED CHANNELS CONFIGURATION FIELDS */
    CFGF_CHAN_IN_TS    = 0,
    CFGF_CHAN_IN_V0_V1 = 1,
    CFGF_CHAN_IN_V2_V3 = 2,
    CFGF_CHAN_IN_TEMP_AMBIENT = 3,
    CFGF_CHAN_IN_TEMP_OBJECT  = 4,
} cfg_field_t;

typedef enum {
    CFG_EV_TIMEOUT   = CFG_EV_TIM | CFGF_TIMEOUT,    /* 0x00 */
    CFG_EV_REPEAT    = CFG_EV_TIM | CFGF_REPEAT,     /* 0x01 */
    CFG_EV_FILEPATH  = CFG_EV_FS | CFGF_FILEPATH,    /* 0x02 */
    CFG_DEV_IIO      = CFG_DEV_TYPE | CFGF_IIO,      /* 0x04 */
    CFG_DEV_IIO_BUFF = CFG_DEV_TYPE | CFGF_IIO_BUFF, /* 0x05 */
    CFG_DEV_NON_IIO  = CFG_DEV_TYPE | CFGF_DEFAULT,  /* 0x06 */
    CFG_BUFF_LEN     = CFG_BUFF | CFGF_BUFF_LEN,     /* 0x08 */
    CFG_BUFF_EN      = CFG_BUFF | CFGF_BUFF_EN,      /* 0x09 */
    CFG_CH_IN_TS            = CFG_MEAS_CHAN | CFGF_CHAN_IN_TS,           /* 0x80 */
    CFG_CH_IN_V0_V1         = CFG_MEAS_CHAN | CFGF_CHAN_IN_V0_V1,        /* 0x81 */
    CFG_CH_IN_V2_V3         = CFG_MEAS_CHAN | CFGF_CHAN_IN_V2_V3,        /* 0x82 */
    CFG_CH_IN_TEMP_AMBIENT  = CFG_MEAS_CHAN | CFGF_CHAN_IN_TEMP_AMBIENT, /* 0x83 */
    CFG_CH_IN_TEMP_OBJECT   = CFG_MEAS_CHAN | CFGF_CHAN_IN_TEMP_OBJECT,  /* 0x84 */
} cfg_t;

typedef struct {
    cfg_t cfg_type;
    char val[64];
} cfgtoken_t;


cfgtoken_t parser_parse(const char *cfg);
const char *parser_get_chan(const cfg_t config);

#endif /* __PARSER_H__ */