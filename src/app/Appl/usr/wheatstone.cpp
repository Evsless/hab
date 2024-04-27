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

#include "utils.h"
#include "wheatstone.h"
#include "hab_device.h"
#include "iio_buffer_ops.h"

#if defined IIO_KMOD_IDX_ADS1115_48 && IIO_KMOD_IDX_ADS1115_48
/**********************************************************************************************************************
 *  PREPROCESSOR DEFINITIONS
 *********************************************************************************************************************/
#define VOLTAGE_THR_LOW  -100
#define VOLTAGE_THR_HIGH 4000

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
    WIPER_INC,
    WIPER_DECR,
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
whtst_node_t *wht_nodes[64];
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

    node = (whtst_node_t *) malloc(sizeof(whtst_node_t));
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

                /* Read initial wiper position */
                (void)read_file(digipot_dev->path.dev_data[0], wiper_buff, sizeof(wiper_buff), MOD_R);
                CROP_NEWLINE(wiper_buff, strlen(wiper_buff));
                node->chan[dgpt].wiper = atoi(wiper_buff);
            }
        }
    }

    return  node;
}

static void wiper_change(whtst_chan_t *chan, wiper_op_t wiper_op) {
    char wiper_buff[8] = {0};
    habdev_t *habdev = chan->digipot;

    if (WIPER_INC == wiper_op) {
        if (chan->wiper + 5 < 1024)
            chan->wiper += 5;
    } else {
        if (chan->wiper - 5 >= 0)
            chan->wiper -= 5;
    }
    
    snprintf(wiper_buff, sizeof(wiper_buff), "%d", chan->wiper);
    write_file(habdev->path.dev_data[0], wiper_buff, sizeof(wiper_buff), MOD_W);
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
    
    
    for (int i = 0; i < node->chan_num; i++)
        node->chan[i].db_count = 0;

    for (int i = 0; i < size; i++) {
        channel = i % node->chan_num;
        if (data_frame[i] > VOLTAGE_THR_HIGH) {
            node->chan[channel].db_count++;
        } else if (data_frame[i] < VOLTAGE_THR_LOW) {
            node->chan[channel].db_count--;
        } else {
            if (node->chan[channel].db_count != 0)
                node->chan[channel].db_count > 0 ? node->chan[channel].db_count-- : node->chan[channel].db_count++;
        }
    }

    for (int i = 0; i < node->chan_num; i++) {
        if (node->chan[i].db_count > 3)
            wiper_change(&node->chan[i], WIPER_INC);
        else if (node->chan[i].db_count < -3)
            wiper_change(&node->chan[i], WIPER_DECR);
    }
}
#endif

/***********************************************************************************************************************
 * END OF FILE
 **********************************************************************************************************************/
