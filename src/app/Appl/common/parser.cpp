#include <string.h>
#include <stdlib.h>

#include "parser.h"
#include "utils.h"

#define S_EV_TIM "tim"
#define S_EV_TIM_TO "to"
#define S_EV_TIM_REP "rep"

#define S_EV_FS "fs"

typedef enum {
    S_BUFF_GROUP = 0,
    S_BUFF_SETTING,
    S_BUFF_VALUE,
} buff_settings_t;

cfgtoken_t parser_buffcfg(const char *cfg) {
    cfgtoken_t token;
    char buff[64] = {0};
    
    u8 token_idx  = 0;
    usize cfg_idx = 0;

    token.val = 1;
    while(get_word(cfg, &cfg_idx, buff, sizeof(buff)) >= 0) {
        switch (token_idx) {
            case S_BUFF_GROUP:
                token.setting_group = buff[0];
                break;
            case S_BUFF_SETTING:
                token.setting[0] = '/'; /* Think of better way of handling that */
                strcpy(token.setting + 1, buff);
                break;
            case S_BUFF_VALUE:
                token.val = atoi(buff);
                break;
            default:
                break;
        }
        token_idx++;
    }

    return token;
}

ev_cfgtoken_t parser_evcfg(const char *cfg) {
    ev_cfgtoken_t token;

    cfg_type_t type;
    cfg_field_t field;

    char buff[64] = {0};

    u8 token_idx = 0;
    usize str_pos = 0;

    while(get_word(cfg, &str_pos, buff, sizeof(buff)) >= 0) {
        if (token_idx == 0) {
            if (0 == str_compare(buff, S_EV_TIM)) {
                type = CFG_EV_TIM;
            }
            else if (0 == str_compare(buff, S_EV_FS))
                type = CFG_EV_FS;
        }

        if (token_idx == 1) {
            if (0 == str_compare(buff, S_EV_TIM_TO))
                field = CFGF_TIMEOUT;
            else if (0 == str_compare(buff, S_EV_TIM_REP))
                field = CFGF_REPEAT;
        }

        if (token_idx == 2) {
            /* IMPORTANT: THINK OF USING VOID POINTER HERE (THIS IS SHITTY FOR NOW) */
            strcpy(token.val, buff);
        }
        token_idx++;
    }
    token.cfg_type = (cfg_t)(type + field);

    return token;
}

