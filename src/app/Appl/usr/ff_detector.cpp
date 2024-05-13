
/**********************************************************************************************************************
 *  INCLUDES
 *********************************************************************************************************************/
#include <string.h>
#include <stdbool.h>

#include "ff_detector.h"
#include "iio_buffer_ops.h"
#include "stdtypes.h"
#include "utils.h"

/**********************************************************************************************************************
 *  PREPROCESSOR DEFINITIONS
 *********************************************************************************************************************/
#define FF_G_THR 500
#define FF_G_LIMIT_LOWER -15
#define FF_G_LIMIT_UPPER 15   /* 0.1 sec */
#define FF_G_FALL_WIN    18   /* 0.5 sec */
#define FF_G_FLIGHT_WIN  200 /* 5.0 sec */

#define PREFALL_DEBOUNCE   1
#define PREFLIGHT_DEBOUNCE -1

#define ACCEL_MG_SCALE 1000
#define ACCEL_2G_PRESC 16384

// #define ACCEL_MG(a) ( \
// { \
//     u64 _val = 0; \
//     for (u8 i = 0; i < 3; i++) \
//         _val += isqr((a + i) * ACCEL_MG_SCALE / ACCEL_2G_PRESC);  \
//     isqrt(_val); \
// })


/**********************************************************************************************************************
 * LOCAL TYPEDEFS DECLARATION
 *********************************************************************************************************************/
typedef enum {
    STATUS_FLIGHT,
    STATUS_PREFALL,
    STATUS_FALL,
    STATUS_PREFLIGHT,
} falling_status_t;

typedef struct {
    falling_status_t fs_stat;
    int fs_cnt;
    int fs_delta;
    u32 fs_sample;
} flight_status_t;

/**********************************************************************************************************************
 * GLOBAL VARIABLES DECLARATION
 *********************************************************************************************************************/

static flight_status_t flight_status = {
    .fs_stat = STATUS_FLIGHT,
    .fs_cnt = 5,
    .fs_delta = 0,
};

/**********************************************************************************************************************
 * LOCAL FUNCTION DECLARATION
 *********************************************************************************************************************/
u32 isqrt(u64 num);
static void flight_handler(void);
static void prefall_handler(void);
static void fall_handler(void);
static void preflight_handler(void);

/**********************************************************************************************************************
 * LOCAL FUNCTION DEFINITION
 *********************************************************************************************************************/
u32 isqrt(u64 num) {
    u32 res = 0, tmp = 0; // result
    u32 add = 1 << 31;
    u64 quad; // 'A^2'

    while ( add > 0 ) {
        tmp = res + add;
        quad = tmp;
        quad *= tmp;
        if ( num >= quad ) {
            res = tmp;
        }
        add >>= 1;
    }
    return res;
}

u32 get_accel_mg(const s64 *data) {
    u64 accel = 0;
    s64 tmp;
    for (u8 i = 0; i < 3; i++) {
        tmp = ((data[i] * (1000L) / (16384L)));
        accel += (1LU) * (tmp * tmp);
    }

    return isqrt(accel);
}

static void flight_handler(void) {
    if ((STATUS_FLIGHT == flight_status.fs_stat) && flight_status.fs_cnt < 0) {
        flight_status.fs_stat = STATUS_PREFALL;
        flight_status.fs_delta = 0;
        printf("PREFALL\n");
    }
}

static void prefall_handler(void) {
    if (STATUS_PREFALL == flight_status.fs_stat) {
        if (flight_status.fs_cnt > PREFALL_DEBOUNCE) {
            flight_status.fs_stat = STATUS_FLIGHT;
            printf("DELTA: %d\n", flight_status.fs_delta++);
        } else {
            if (flight_status.fs_sample < FF_G_THR)
                flight_status.fs_delta++;
            else if (flight_status.fs_sample > FF_G_THR && flight_status.fs_delta > 0)
                flight_status.fs_delta--;

            if (FF_G_FALL_WIN == flight_status.fs_delta) {
                flight_status.fs_stat = STATUS_FALL;
                printf("FALLING\n");
            }
        }
    }
}

static void fall_handler(void) {
    if (STATUS_FALL == flight_status.fs_stat) {
        if (flight_status.fs_cnt > 0) {
            flight_status.fs_stat = STATUS_PREFLIGHT;
            flight_status.fs_delta = 0;
        }
    }
}

static void preflight_handler(void) {
    if (STATUS_PREFLIGHT == flight_status.fs_stat) {
        if (flight_status.fs_cnt < -1) {
            flight_status.fs_stat = STATUS_FALL;
        } else {
            flight_status.fs_delta++;
            if (FF_G_FLIGHT_WIN == flight_status.fs_delta)
                flight_status.fs_stat = STATUS_FLIGHT;
        }
    }
}

/**********************************************************************************************************************
 * GLOBAL FUNCTION DEFINITION
 *********************************************************************************************************************/
void ffdet_process_frame(const habdev_t *accel_dev) {
    u8 raw_data_buff[4096]  = {0};
    s64 data_buff[2048] = {0};
    char path_buff[128] = {0};
    char data_available_buff[8] = {0};
    int raw_data_samples = 0, accel_samples = 0;

    /* REWRITE TO CORRECTLY EXTRACT BUFFER PATH */
    // snprintf(path_buff, sizeof(path_buff), "%s/buffer/data_available", accel_dev->path.dev_path);
    (void)read_file(path_buff, data_available_buff, sizeof(data_available_buff), MOD_R);
    CROP_NEWLINE(data_available_buff, strlen(data_available_buff));

    raw_data_samples = min(4092, atoi(data_available_buff) * 6);

    /* REWRITE THE DEVICE DATA PATH */
    // (void)read_file(accel_dev->path.dev_data[0], (char *)raw_data_buff, raw_data_samples, MOD_RW);
    accel_samples = iiobuff_extract_data(accel_dev->df, data_buff, raw_data_buff, raw_data_samples);
    printf("accel_smpl: %d\n", raw_data_samples);
    // printf("----------FRAME-------------\n");

    for (int i = 0; i < accel_samples; i += 3 ) {
        flight_status.fs_sample = get_accel_mg(data_buff + i);
        // printf("%u\n", flight_status.fs_sample);
        // printf("%d %d %d\n", data_buff[i], data_buff[i + 1], data_buff[i + 2]);

        if ((flight_status.fs_sample <= FF_G_THR) && (flight_status.fs_cnt > FF_G_LIMIT_LOWER))
            flight_status.fs_cnt--;
        else if ((flight_status.fs_sample > FF_G_THR) && (flight_status.fs_cnt < FF_G_LIMIT_UPPER))
            flight_status.fs_cnt++;
        
        switch (flight_status.fs_stat) {
            case STATUS_FLIGHT:
                flight_handler();
                break;
            case STATUS_PREFALL:
                prefall_handler();
                break;
            case STATUS_FALL:
                fall_handler();
                break;
            case STATUS_PREFLIGHT:
                preflight_handler();
                break;
            default:
                break;
        }

        printf("status: %d, counter: %d, delta: %d, val: %d\n", flight_status.fs_stat, flight_status.fs_cnt, flight_status.fs_delta, flight_status.fs_sample);
    }
    // printf("-----FRAME_END------\n");
}

/***********************************************************************************************************************
 * END OF FILE
 **********************************************************************************************************************/
