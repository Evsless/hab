#ifndef __CFG_TREE_H__
#define __CFG_TREE_H__

#include <stdio.h>
#include <stdbool.h>

#include "llist.h"

#define DEV_CONFIG_DEFAULT      0x00     
#define DEV_CONFIG_REG_VAL      0x02
#define DEV_CONFIG_REG_NAME     0x03
#define DEV_CONFIG_REG_SCHAN    0x01 << 4
#define DEV_CONFIG_REG_TIM_TO   0x02 << 4
#define DEV_CONFIG_REG_TIM_REP  0x03 << 4
#define DEV_CONFIG_REG_LEN      0x04 << 4
#define DEV_CONFIG_REG_ENABLE   0x05 << 4
#define DEV_CONFIG_REG_PATH     0x06 << 4
#define DEV_CONFIG_REG_CMD      0x07 << 4
#define DEV_CONFIG_REG_BUFF     0x01 << 12
#define DEV_CONFIG_REG_CHAN     0x02 << 12
#define DEV_CONFIG_REG_BUFF_CH  0x03 << 12
#define DEV_CONFIG_REG_EVENT    0x04 << 12
#define DEV_CONFIG_REG_CAM      0x05 << 12
#define DEV_CONFIG_REG_PARAM    0x06 << 12

#define DEV_CONFIG_REG_DEVTYPE_DEFAULT 0x00
#define DEV_CONFIG_REG_DEVTYPE_IIO     0x01 << 16
#define DEV_CONFIG_REG_DEVTYPE_IIOBUFF 0x02 << 16
#define DEV_CONFIG_REG_DEVTYPE_CAMERA  0x03 << 16

#define CFGTREE_BUFF_CONFIG             (DEV_CONFIG_REG_BUFF)
#define CFGTREE_BUFF_ENABLE             (CFGTREE_BUFF_CONFIG | (DEV_CONFIG_REG_ENABLE) | (DEV_CONFIG_REG_VAL))
#define CFGTREE_CHAN_NAME_CONFIG        ((DEV_CONFIG_REG_CHAN) | (DEV_CONFIG_REG_SCHAN) | (DEV_CONFIG_REG_NAME))
#define CFGTREE_CHAN_VAL_CONFIG         ((DEV_CONFIG_REG_CHAN) | (DEV_CONFIG_REG_SCHAN) | (DEV_CONFIG_REG_VAL))
#define CFGTREE_BUFF_CHAN_NAME_CONFIG   ((CFGTREE_CHAN_NAME_CONFIG) | (CFGTREE_BUFF_CONFIG))
#define CFGTREE_BUFF_CHAN_VAL_CONFIG    ((CFGTREE_CHAN_VAL_CONFIG)  | (CFGTREE_BUFF_CONFIG))
#define CFGTREE_BUFF_LEN_CONFIG         ((CFGTREE_BUFF_CONFIG) | (DEV_CONFIG_REG_LEN) | (DEV_CONFIG_REG_VAL))
#define CFGTREE_EVENT                   (DEV_CONFIG_REG_EVENT)
#define CFGTREE_EVENT_TIM_TO_CONFIG     ((DEV_CONFIG_REG_EVENT) | (DEV_CONFIG_REG_TIM_TO) | (DEV_CONFIG_REG_VAL))
#define CFGTREE_EVENT_TIM_REP_CONFIG    ((DEV_CONFIG_REG_EVENT) | (DEV_CONFIG_REG_TIM_REP) | (DEV_CONFIG_REG_VAL))

typedef enum {
    ST_DEFAULT,
    ST_PRECONF,
    ST_CONF,
    ST_CONF_CLOSE,
    ST_SAVE_CONF,
    ST_VAL,
    ST_SAVE_VAL,
    ST_CONF_RDY,
} states_t;

typedef enum {
    CFG_IIO_DEV,
    CFG_IIO_BUFF_DEV,
    CFG_CAMERA_DEV,
    CFG_DEFAULT_DEV,
    CFG_BUFF_T,
    CFG_EVENT,
    CFG_CHANNELS,
    CFG_TIM_TO,
    CFG_TIM_REP,
    CFG_SINGLE_CHAN,
    CFG_FIELD_NAME,
    CFG_FIELD_VAL,
    CFG_BUFF_LEN_T,
    CFG_BUFF_ENABLE,
    CFG_TYPE_NUM,
} cfg_type_tree_t;

typedef struct node {
    cfg_type_tree_t type;
    char val[64];
    struct node *child[8];
    int child_num;
} node_t;


typedef struct
{
    bool is_close;
    bool has_data;
    cfg_type_tree_t cfg;
    char data[64];
} xml_token_t;


node_t *cfgtree_init(xml_line_t **line);
void cfgtree_free(node_t *root);

void cfgtree_initDfa(void);
void cfgtree_freeDfa(void);
void cfgtree_dfaReset(void);

node_t *cfgtree_getNode(node_t *root, const cfg_type_tree_t cfg);
int cfgtree_getCfgReg(const int conf);

#endif /* __CFG_TREE_H__ */
