/**********************************************************************************************************************
 *  INCLUDES
 *********************************************************************************************************************/
#include <string.h>
#include <stdlib.h>

#include "dfa.h"
#include "utils.h"
#include "cfg_tree.h"

/**********************************************************************************************************************
 *  PREPROCESSOR DEFINITIONS
 *********************************************************************************************************************/
#define _parse get_cfg

/**********************************************************************************************************************
 * LOCAL TYPEDEFS DECLARATION
 *********************************************************************************************************************/
typedef struct cfgtype_ht
{
    const char *key;
    cfg_type_tree_t val;
} cfgtype_ht_t;

typedef struct cfgreg_ht {
    const int key;
    const int val;
} cfgreg_ht_t;

typedef struct cfgpath_ht {
    const int key;
    const char *val;
} cfgpath_ht_t;


/**********************************************************************************************************************
 * GLOBAL VARIABLES DECLARATION
 *********************************************************************************************************************/
dfa_t *xml_dfa = NULL;

cfgtype_ht_t cfgtype_lut[] = {
    {"iio_dev",     CFG_IIO_DEV},
    {"iio_buff_dev", CFG_IIO_BUFF_DEV},
    {"camera_dev",  CFG_CAMERA_DEV},
    {"ev_global",   CFG_EV_GLOBAL},
    {"index",       CFG_INDEX},
    {"buff",        CFG_BUFF_T},
    {"channels",    CFG_CHANNELS},
    {"cam",         CFG_CAM},
    {"still",       CFG_CAM_STILL},
    {"vid",         CFG_CAM_VIDEO},
    {"chan",        CFG_SINGLE_CHAN},
    {"name",        CFG_FIELD_NAME},
    {"val",         CFG_FIELD_VAL},
    {"buff_len",    CFG_BUFF_LEN_T},
    {"enable",      CFG_BUFF_ENABLE},
    {"event",       CFG_EVENT},
    {"tim_to",      CFG_TIM_TO},
    {"tim_rep",     CFG_TIM_REP},
    {"global_ev_ref", CFG_EV_GLOBAL_REF},
};

cfgreg_ht_t cfgreg_lut[] = {
    {CFG_IIO_DEV,       DEV_CONFIG_REG_DEVTYPE_IIO},
    {CFG_IIO_BUFF_DEV,  DEV_CONFIG_REG_DEVTYPE_IIOBUFF},
    {CFG_CAMERA_DEV,    DEV_CONFIG_REG_DEVTYPE_CAMERA},
    {CFG_EV_GLOBAL,     DEV_CONFIG_REG_GLOBAL_EVENT},
    {CFG_INDEX,         DEV_CONFIG_REG_INDEX},
    {CFG_BUFF_T,        DEV_CONFIG_REG_BUFF},
    {CFG_BUFF_LEN_T,    DEV_CONFIG_REG_LEN},
    {CFG_BUFF_ENABLE,   DEV_CONFIG_REG_ENABLE},
    {CFG_CHANNELS,      DEV_CONFIG_REG_CHAN},
    {CFG_SINGLE_CHAN,   DEV_CONFIG_REG_SCHAN},
    {CFG_CAM,           DEV_CONFIG_REG_CAM},
    {CFG_CAM_STILL,     DEV_CONFIG_REG_CAM_ST},
    {CFG_CAM_VIDEO,     DEV_CONFIG_REG_CAM_VID},
    {CFG_EVENT,         DEV_CONFIG_REG_EVENT},
    {CFG_TIM_TO,        DEV_CONFIG_REG_TIM_TO},
    {CFG_TIM_REP,       DEV_CONFIG_REG_TIM_REP},
    {CFG_FIELD_NAME,    DEV_CONFIG_REG_NAME},
    {CFG_FIELD_VAL,     DEV_CONFIG_REG_VAL},
    {CFG_EV_GLOBAL_REF, DEV_CONFIG_REG_EV_G_REF},
};

static char cfg_buffer[128];
static char last_sym;
xml_token_t token = {0};

/**********************************************************************************************************************
 * LOCAL FUNCTION DECLARATION
 *********************************************************************************************************************/
static cfg_type_tree_t str2cfg(const char *str_cfg);
static void xml_action(const int state);

static void set_token(xml_token_t val);
static void clean_token(void);
static xml_token_t get_token(void);

static xml_token_t get_cfg(const char *line, const int size);

/**********************************************************************************************************************
 * LOCAL FUNCTION DEFINITION
 *********************************************************************************************************************/
static cfg_type_tree_t str2cfg(const char *str_cfg) {
    cfg_type_tree_t retval = (cfg_type_tree_t)0;

    for (int i = 0; i < (sizeof(cfgtype_lut) / sizeof(cfgtype_ht_t)); i++) {
        if (0 == strcmp(str_cfg, cfgtype_lut[i].key)) {
            retval = cfgtype_lut[i].val;
            break;
        }
    }

    return retval;
}

void xml_action(const int state) {
    xml_token_t local_token = get_token();

    switch (state)
    {
    case ST_CONF:
        strncat(cfg_buffer, &last_sym, 1);
        break;

    case ST_SAVE_CONF:
        local_token.cfg = str2cfg(cfg_buffer);
        memset(cfg_buffer, 0, sizeof(cfg_buffer));
        break;

    case ST_VAL:
        strncat(cfg_buffer, &last_sym, 1);
        break;

    case ST_SAVE_VAL:
        local_token.has_data = true;
        strcpy(local_token.data, cfg_buffer);
        memset(cfg_buffer, 0, sizeof(cfg_buffer));
        break;

    case ST_CONF_CLOSE:
        local_token.is_close = true;
        break;

    default:
        break;
    }

    set_token(local_token);
}

static xml_token_t get_token(void) {
    return token;
}

static void set_token(xml_token_t val) {
    token = val;
}

static void clean_token(void) {
    memset(&token, 0, sizeof(token));
}

/**********************************************************************************************************************
 * GLOBAL FUNCTION DEFINITION
 *********************************************************************************************************************/
void cfgtree_initDfa(void) {
    xml_dfa = dfa_init();

    dfa_addState(xml_dfa, ST_DEFAULT, false, NULL);
    dfa_addState(xml_dfa, ST_PRECONF, false, NULL);
    dfa_addState(xml_dfa, ST_CONF, true, &xml_action);
    dfa_addState(xml_dfa, ST_SAVE_CONF, true, &xml_action);
    dfa_addState(xml_dfa, ST_VAL, true, &xml_action);
    dfa_addState(xml_dfa, ST_SAVE_VAL, true, &xml_action);
    dfa_addState(xml_dfa, ST_CONF_CLOSE, true, &xml_action);
    dfa_addState(xml_dfa, ST_CONF_RDY, false, NULL);

    dfa_addTransition(xml_dfa, ST_DEFAULT, ST_PRECONF, '<', CON_EQ);
    dfa_addTransition(xml_dfa, ST_PRECONF, ST_CONF, '/', CON_NEQ);
    dfa_addTransition(xml_dfa, ST_PRECONF, ST_CONF_CLOSE, '/', CON_EQ);
    dfa_addTransition(xml_dfa, ST_CONF, ST_SAVE_CONF, '>', CON_EQ);
    dfa_addTransition(xml_dfa, ST_SAVE_CONF, ST_VAL, '\n', CON_NEQ);
    dfa_addTransition(xml_dfa, ST_SAVE_CONF, ST_CONF_RDY, '\n', CON_EQ);
    dfa_addTransition(xml_dfa, ST_VAL, ST_SAVE_VAL, '<', CON_EQ);
    dfa_addTransition(xml_dfa, ST_SAVE_VAL, ST_CONF_CLOSE, '/', CON_EQ);
    dfa_addTransition(xml_dfa, ST_CONF_CLOSE, ST_CONF, '>', CON_NEQ);
    dfa_addTransition(xml_dfa, ST_CONF_CLOSE, ST_CONF_RDY, '\n', CON_EQ);

}

void cfgtree_freeDfa(void) {
    dfa_free(xml_dfa);
}

void cfgtree_dfaReset(void) {
    dfa_setStartState(xml_dfa, ST_DEFAULT);
}


xml_token_t get_cfg(const char *line, const int size) {
    clean_token();
    if (NULL == line)
        return token;

    for (int i = 0; i < size; i++) {
        last_sym = line[i];
        dfa_transition(xml_dfa, last_sym);
        if (xml_dfa->states[xml_dfa->current_state]->has_action)
            xml_dfa->states[xml_dfa->current_state]->action(xml_dfa->current_state);
    }

    /* After parsing the line reset the state machine */
    xml_dfa->current_state = ST_DEFAULT;

    return token;
}

node_t *cfgtree_init(xml_line_t **line) {
    node_t *new_node = NULL;

    xml_token_t cfg_curr = {0};
    xml_token_t cfg_next = {0};
    bool new_child = true;

    cfg_curr = get_cfg((*line)->val, strlen((*line)->val));

    if (!cfg_curr.is_close || cfg_curr.has_data) {
        new_node = (node_t *)calloc(1, sizeof(node_t));
        new_node->type = cfg_curr.cfg;

        if (cfg_curr.has_data)
            strcat(new_node->val, cfg_curr.data);

        while (new_child) {
            if (NULL == (*line)->next)
                break;

            cfg_next = _parse((*line)->next->val, strlen((*line)->next->val));
            if (cfg_next.cfg != cfg_curr.cfg) {
                if (!cfg_curr.has_data) {
                    (*line) = (*line)->next;
                    new_node->child[new_node->child_num++] = cfgtree_init(line);
                } else {
                    new_child = false;
                }
            } else {
                (*line) = (*line)->next;
                new_child = false;
            }
        }
    }

    return new_node;
}

node_t *cfgtree_getNode(node_t *root, const cfg_type_tree_t cfg) {
    node_t *retval = root;
    int cn_cnt = 0;

    while (cfg != retval->type && cn_cnt < retval->child_num)
        retval = cfgtree_getNode(retval->child[cn_cnt++], cfg);

    return retval;
}

void cfgtree_free(node_t *root) {
    int child_cnt = 0;

    while (child_cnt < root->child_num) {
        cfgtree_free(root->child[child_cnt++]);
    }

    free(root);
}

int cfgtree_getCfgReg(const int conf) {
    int retval = 0;

    for (int i = 0; i < CFG_TYPE_NUM; i++) {
        if (conf == cfgreg_lut[i].key) {
            retval = cfgreg_lut[i].val;
            break;
        }
    }

    return retval;
}

/***********************************************************************************************************************
 * END OF FILE
 **********************************************************************************************************************/
