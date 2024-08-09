/**********************************************************************************************************************
* wheatstone.cpp                                                                                                      *
***********************************************************************************************************************
* DESCRIPTION :                                                                                                       *
*       Application specific code called from callback. Contains and API for processing ADC readouts                  *
*       in pair with controlling the digital potentiometers attached to them.                                         *
*                                                                                                                     *
* PUBLIC FUNCTIONS :                                                                                                  *
*       void                wheatstone_run(const habdev_t *adc_dev)                                                   *
*                                                                                                                     *
* AUTHOR :                                                                                                            *
*       Yahor Yauseyenka    email: yahoryauseyenka@gmail.com                                                          *
*                                                                                                                     *
* VERSION                                                                                                             *
*       0.0.1               last modification: 23-04-2024                                                             *
*                                                                                                                     *
* LICENSE                                                                                                             *
*       GPL                                                                                                           *
*                                                                                                                     *
***********************************************************************************************************************/

/**********************************************************************************************************************
 *  INCLUDES
 *********************************************************************************************************************/
#include <string.h>
#include <stdio.h>

#include "utils.h"
#include "wheatstone.h"
#include "hab_device.h"
#include "iio_buffer_ops.h"

#if (defined IIO_KMOD_IDX_ADS1115_48) && (defined IIO_KMOD_IDX_ADS1115_48)

/**********************************************************************************************************************
 *  PREPROCESSOR DEFINITIONS
 *********************************************************************************************************************/
#define POT_RESOLUTION   10u
#define DEB_THRESHOLD    1

#define WIPER_STEP       1
#define WIPER_MAX_POS    (0x01 << POT_RESOLUTION)
#define WIPER_MIN_POS    0

#define VOLTAGE_THR_LOW  -1000
#define VOLTAGE_THR_HIGH 1000

#define UINT16_MAX_VAL 32767U
#define UINT16_PRESC   65536

/**********************************************************************************************************************
 * LOCAL TYPEDEFS DECLARATION
 *********************************************************************************************************************/
typedef struct {
    int adc_dev;
    int chan_num;
    int digipot_dev[8];
} whtst_setup_t;

typedef struct {
    int wiper;
    int db_count;
    habdev_t *digipot;
} whtst_chan_t;

typedef struct {
    int adc_id;
    u8 chan_num;
    whtst_chan_t chan[8];
} whtst_node_t;

typedef enum {
    WIPER_OP_INC,
    WIPER_OP_DECR,
} wiper_op_t;

/* Below array shall be code generated (in ideal world) */
whtst_setup_t setup[] = {
    {
        .adc_dev = IIO_KMOD_IDX_ADS1115_48,
        .chan_num = 2,
        .digipot_dev = {
            IIO_KMOD_IDX_AD5272_2E,
            IIO_KMOD_IDX_AD5272_2C,
        },
    },
    {
        .adc_dev = IIO_KMOD_IDX_ADS1115_49,
        .chan_num = 1,
        .digipot_dev = {
            IIO_KMOD_IDX_AD5272_2F, 
        },
    },
};

/**********************************************************************************************************************
 * GLOBAL VARIABLES DECLARATION
 *********************************************************************************************************************/
whtst_node_t *wht_nodes[64] = {0};
static int wht_node_cnt;

/**********************************************************************************************************************
 * LOCAL FUNCTION DECLARATION
 *********************************************************************************************************************/
static whtst_node_t *get_wht_node(const int adc_index);
static whtst_node_t *wheatstone_init(const int adc_index);
static void wiper_change(whtst_chan_t *chan, wiper_op_t wiper_op);


/**********************************************************************************************************************
 * LOCAL FUNCTION DEFINITION
 *********************************************************************************************************************/
static whtst_node_t *get_wht_node(const int adc_index) {
    whtst_node_t *node = NULL;
    for (int i = 0; wht_nodes[i] != NULL; i++) {
        if (adc_index == wht_nodes[i]->adc_id) {
            node = wht_nodes[i];
            break;
        }
    }
    
    return node;
}

static whtst_node_t *wheatstone_init(const int adc_index) {
    whtst_node_t *node = NULL;
    habdev_t *digipot_dev = NULL;
    char wiper_buff[8];
    char dev_path[128] = {0};

    node = (whtst_node_t *) malloc(sizeof(whtst_node_t));
    memset(node, 0, sizeof(whtst_node_t));
    if (NULL != node) {
        wht_nodes[wht_node_cnt++] = node;
    } else {
        fprintf(stderr, "ERROR: Error allocation wheatstone device.\n");
        return NULL;
    }

    node->adc_id = adc_index;
    for (int i = 0; i < ARRAY_SIZE(setup); i++) {
        if (setup[i].adc_dev == node->adc_id) {
            node->chan_num = setup[i].chan_num;
            for (int dgpt = 0; dgpt < node->chan_num; dgpt++) {
                digipot_dev = habdev_get(setup[i].digipot_dev[dgpt]);
                node->chan[dgpt].digipot = digipot_dev;

                habdev_getDevPath(digipot_dev, dev_path, sizeof(dev_path));
                strcat(dev_path, digipot_dev->path.channel[0]);

                /* Read initial wiper position */
                (void)read_file(dev_path, wiper_buff, sizeof(wiper_buff), MOD_R);
                CROP_NEWLINE(wiper_buff, strlen(wiper_buff));
                node->chan[dgpt].wiper = atoi(wiper_buff);
            }
        }
    }

    return  node;
}

static void wiper_change(whtst_chan_t *chan, wiper_op_t wiper_op) {
    char wiper_buff[8] = {0};
    char dev_path[128] = {0};
    habdev_t *habdev = chan->digipot;

    habdev_getDevPath(habdev, dev_path, sizeof(dev_path));
    strcat(dev_path, habdev->path.channel[0]);

    if (WIPER_OP_INC == wiper_op) {
        if (chan->wiper + WIPER_STEP < WIPER_MAX_POS)
            chan->wiper += WIPER_STEP;
    } else {
        if (chan->wiper - WIPER_STEP >= WIPER_MIN_POS)
            chan->wiper -= WIPER_STEP;
    }
    
    snprintf(wiper_buff, sizeof(wiper_buff), "%d", chan->wiper);
    write_file(dev_path, wiper_buff, sizeof(wiper_buff), MOD_W);
}

/**********************************************************************************************************************
 * GLOBAL FUNCTION DEFINITION
 *********************************************************************************************************************/
void wheatstone_run(const habdev_t *adc_dev) {
    whtst_node_t *node = get_wht_node(adc_dev->index);
    
    int size, channel = 0;
    u8   data_raw[4096]     = {0};
    s64  data_frame[1024]   = {0};
    char wiper_pos_buff[16] = {0};

    if (NULL == node)
        node = wheatstone_init(adc_dev->index);

    for (int i = 0; i < node->chan_num; i++)
        snprintf(wiper_pos_buff + strlen(wiper_pos_buff), sizeof(wiper_pos_buff), 
            "%d%c", node->chan[i].wiper, (i == node->chan_num - 1) ? '\0' : ' ');
    
    size = iiobuff_log2file(adc_dev, wiper_pos_buff, data_raw);
    size = iiobuff_extract_data(adc_dev->df, data_frame, data_raw, size);
    
    // for (int i = 0; i < node->chan_num; i++)
    //     node->chan[i].db_count = 0;

    // for (int i = 0; i < size; i++) {
    //     channel = i % node->chan_num;
    //     if (data_frame[i] > VOLTAGE_THR_HIGH) {
    //         node->chan[channel].db_count++;
    //     } else if (data_frame[i] < VOLTAGE_THR_LOW) {
    //         node->chan[channel].db_count--;
    //     } else {
    //         if (node->chan[channel].db_count != 0)
    //             node->chan[channel].db_count > 0 ? node->chan[channel].db_count-- : node->chan[channel].db_count++;
    //     }
    // }

    // for (int i = 0; i < node->chan_num; i++) {
    //     if (node->chan[i].db_count > DEB_THRESHOLD)
    //         wiper_change(&node->chan[i], WIPER_OP_DECR);
    //     else if (node->chan[i].db_count < -DEB_THRESHOLD)
    //         wiper_change(&node->chan[i], WIPER_OP_INC);
    // }
}

void wheatstone_runSingleChan(const habdev_t *adc_dev) {
    whtst_node_t *node = get_wht_node(adc_dev->index);

    int chan_val = 0;
    char dev_path[128] = {0};
    char data_buffer[16] = {0};

    if (NULL == node)
        node = wheatstone_init(adc_dev->index);

    for (int i = 0; i < node->chan_num; i++) {
        habdev_getDevPath(adc_dev, dev_path, sizeof(dev_path));
        strcat(dev_path, adc_dev->path.channel[i]);

        (void)read_file(dev_path, data_buffer, sizeof(data_buffer), MOD_R);
        CROP_NEWLINE(data_buffer, strlen(data_buffer));
        chan_val = atoi(data_buffer);

        if (chan_val > UINT16_MAX_VAL)
            chan_val -= UINT16_PRESC;
        
        if (chan_val > VOLTAGE_THR_HIGH) {
            if (node->chan[i].db_count < 5)
                node->chan[i].db_count++;
        } else if (chan_val < VOLTAGE_THR_LOW) {
            if (node->chan[i].db_count > -5)
                node->chan[i].db_count--;
        } else {
            if (node->chan[i].db_count != 0)
                node->chan[i].db_count > 0 ? node->chan[i].db_count-- : node->chan[i].db_count++;
        }

        if (node->chan[i].db_count > 2)
            wiper_change(&node->chan[i], WIPER_OP_DECR);
        else if (node->chan[i].db_count < -2)
            wiper_change(&node->chan[i], WIPER_OP_INC);
    }
}

#endif

/***********************************************************************************************************************
 * END OF FILE
 **********************************************************************************************************************/
