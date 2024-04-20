/**********************************************************************************************************************
* parser.cpp                                                                                                          *
***********************************************************************************************************************
* DESCRIPTION :                                                                                                       *
*       Module that defines APIs for extracting the data from HAB configuration files.                                *
*                                                                                                                     *
* PUBLIC FUNCTIONS :                                                                                                  *
*       cfgtoken_t          parser_parse(const char *cfg)                                                             *
*       const char*         parser_get_chan(const cfg_t config)                                                       *
*                                                                                                                     *
* AUTHOR :                                                                                                            *
*       Yahor Yauseyenka    email: yahoryauseyenka@gmail.com                                                          *
*                                                                                                                     *
* VERSION                                                                                                             *
*       0.0.4               last modification: 20-04-2024                                                             *
*                                                                                                                     *
* LICENSE                                                                                                             *
*       GPL                                                                                                           *
*                                                                                                                     *
***********************************************************************************************************************/

/**********************************************************************************************************************
 *  INCLUDES
 *********************************************************************************************************************/
#include <string.h>
#include <stdlib.h>

#include "parser.h"
#include "utils.h"

/**********************************************************************************************************************
 *  PREPROCESSOR DEFINITIONS
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * LOCAL TYPEDEFS DECLARATION
 *********************************************************************************************************************/
typedef struct {
    const char *key;
    int val;
} ht_elem_t;


/**********************************************************************************************************************
 * GLOBAL VARIABLES DECLARATION
 *********************************************************************************************************************/
static const ht_elem_t tokens[] = {
    /* TIMER EVENT CONFIG */
    {"tim", CFG_EV_TIM},
    {"to", CFGF_TIMEOUT},
    {"rep", CFGF_REPEAT},
    /* FILE SYSTEM EVENT CONFIG */
    {"fs", CFG_EV_FS},
    {"fpath", CFGF_FILEPATH},
    /* DEVICE TYPE CONFIG */
    {"type", CFG_DEV_TYPE},
    {"iio", CFGF_IIO},
    {"iio_buff", CFGF_IIO_BUFF},
    {"default", CFGF_DEFAULT},
    /* BUFFER CONFIG */
    {"buf", CFG_BUFF},
    {"length", CFGF_BUFF_LEN},
    {"enable", CFGF_BUFF_EN},
    /* MEASURED CHANNELS CONFIG */
    {"ch", CFG_MEAS_CHAN},
    {"in_timestamp_en", CFGF_CHAN_IN_TS},
    {"in_voltage0-voltage1_en", CFGF_CHAN_IN_V0_V1},
    {"in_voltage2-voltage3_en", CFGF_CHAN_IN_V2_V3},
    {"in_temp_ambient_en", CFGF_CHAN_IN_TEMP_AMBIENT},
    {"in_temp_object_en", CFGF_CHAN_IN_TEMP_OBJECT},
};

static const char* channels[] = {
    "in_timestamp_en",              /* CFG_CH_IN_TS */
    "in_voltage0-voltage1_en",      /* CFG_CH_IN_V0_V1 */
    "in_voltage2-voltage3_en",      /* CFG_CH_IN_V2_V3 */
    "in_temp_ambient_en",           /* CFG_CH_IN_TEMP_AMBIENT */
    "in_temp_object_en",            /* CFG_CH_IN_TEMP_OBJECT */
};

/**********************************************************************************************************************
 * LOCAL FUNCTION DEFINITION
 *********************************************************************************************************************/
static int get_ht_val(const char *key) {
    int ret = -1;
    usize i = 0;

    for (i = 0; i < ARRAY_SIZE(tokens); i++) {
        if (0 == str_compare(key, tokens[i].key))
            return tokens[i].val;
    }

    return ret;
}

/**********************************************************************************************************************
 * GLOBAL FUNCTION DEFINITION
 *********************************************************************************************************************/
cfgtoken_t parser_parse(const char *cfg) {
    cfgtoken_t token;
    cfg_type_t type;
    cfg_field_t field;
    char word[64] = {0};
    
    u8 token_idx  = 0;
    usize cfg_idx = 0;

    while(get_word(cfg, &cfg_idx, word, sizeof(word)) >= 0) {
        switch (token_idx) {
            case 0:
                type = (cfg_type_t) get_ht_val(word);
                break;
            case 1:
                field = (cfg_field_t) get_ht_val(word);
                break;
            case 2:
                memcpy(token.val, word, sizeof(word));
                break;
            default:
                break;
        }
        token_idx++;
    }

    token.cfg_type = (cfg_t)(type | field);
    return token;
}

const char *parser_get_chan(const cfg_t config) {
    const char *ret;
    int idx = config - 128;
    
    if (idx >= 0)
        ret = channels[idx];
    else
        ret = NULL;

    return ret; 
}

/***********************************************************************************************************************
 * END OF FILE
 **********************************************************************************************************************/
