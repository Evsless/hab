#ifndef __PARSER_H__
#define __PARSER_H__

#include "stdtypes.h"

typedef enum {
    CFG_EV_TIM = 0,
    CFG_EV_FS = 10,
} cfg_type_t;

typedef enum {
    CFGF_TIMEOUT = 0,
    CFGF_REPEAT,
    CFGF_FILEPATH,
} cfg_field_t;

typedef enum {
    CFG_EV_TIMEOUT = CFG_EV_TIM + CFGF_TIMEOUT,
    CFG_EV_REPEAT = CFG_EV_TIM + CFGF_REPEAT,
    CFG_EV_FILEPATH = CFG_EV_FS + CFGF_FILEPATH,
} cfg_t;

typedef struct {
    char setting_group;
    char setting[64];
    u32 val;
} cfgtoken_t;

typedef struct {
    cfg_t cfg_type;
    char val[64];
} ev_cfgtoken_t;

cfgtoken_t parser_buffcfg(const char *cfg);
ev_cfgtoken_t parser_evcfg(const char *cfg);

#endif /* __PARSER_H__ */