/**********************************************************************************************************************
* hab_trig.cpp                                                                                                        *
***********************************************************************************************************************
* DESCRIPTION :                                                                                                       *
*       Module for creating and registering triggers, that later will be used                                         *
*       for triggering iio buffer.                                                                                    *
*                                                                                                                     *
* PUBLIC FUNCTIONS :                                                                                                  *
*       habtrig_t*          habtrig_alloc(void);                                                                      *
*       void                habtrig_free(habtrig_t *trig);                                                            *
*       stdret_t            habtrig_register(habtrig_t *trig, u32 period_ms);                                         *
*                                                                                                                     *
* AUTHOR :                                                                                                            *
*       Yahor Yauseyenka    email: yahoryauseyenka@gmail.com                                                          *
*                                                                                                                     *
* VERSION                                                                                                             *
*       0.0.1               last modification: 13-04-2024                                                             *
*                                                                                                                     *
* LICENSE                                                                                                             *
*       GPL                                                                                                           *
*                                                                                                                     *
***********************************************************************************************************************/

/**********************************************************************************************************************
 *  INCLUDES
 *********************************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "hab_trig.h"
#include "utils.h"

/**********************************************************************************************************************
 *  PREPROCESSOR DEFINITIONS
 *********************************************************************************************************************/
#define IIO_HRTRIG_CONFIGFS_PATH "/config/iio/triggers/hrtimer/"
#define IIO_HRTRIG_SYSFS_PATH    "/sys/bus/iio/devices/"
#define IIO_HRTRIG_SF_SUBPATH    "/sampling_frequency"
#define IIO_HRTRIG_BASENAME      "trigger"

#define HAB_HRTRIG_BASENAME      "habtrig-"

/**********************************************************************************************************************
 * GLOBAL VARIABLES DECLARATION
 *********************************************************************************************************************/
static habtrig_t *habtrig_list[64];
static int       habtrig_cnt;

/**********************************************************************************************************************
 * LOCAL FUNCTION DECLARATION
 *********************************************************************************************************************/
static stdret_t write_period(const habtrig_t *trig, u32 period_ms);

/**********************************************************************************************************************
 * LOCAL FUNCTION DEFINITION
 *********************************************************************************************************************/
static stdret_t write_period(const habtrig_t *trig, u32 period_ms) {
    stdret_t ret = STD_NOT_OK;
    float freq = 1 * MILLI / (float)period_ms;

    char path_buff[128] = {0};
    char write_buff[16]  = {0};

    snprintf(path_buff, sizeof(path_buff), "%s%s%d%s", 
                IIO_HRTRIG_SYSFS_PATH, IIO_HRTRIG_BASENAME, trig->index, IIO_HRTRIG_SF_SUBPATH);
    snprintf(write_buff, sizeof(write_buff), "%f", freq);

    ret = write_file(path_buff, write_buff, sizeof(write_buff), MOD_W);

    return ret;
}

/**********************************************************************************************************************
 * GLOBAL FUNCTION DEFINITION
 *********************************************************************************************************************/
stdret_t habtrig_register(habtrig_t *trig, u32 period_ms) {
    stdret_t ret  = STD_NOT_OK;
    int mkdir_ret = 0;

    char trig_path[128]  = {0};

    snprintf(trig_path, sizeof(trig_path), "%s%s%d", IIO_HRTRIG_CONFIGFS_PATH, HAB_HRTRIG_BASENAME, period_ms);

    mkdir_ret = mkdir(trig_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    if (mkdir_ret >= 0) {
        trig->period_ms = period_ms;
        trig->type = T_HRTIM;
        snprintf(trig->name, sizeof(trig->name), "%s%d", HAB_HRTRIG_BASENAME, trig->period_ms);

        ret = write_period(trig, period_ms);
    }

    return ret;
}


habtrig_t *habtrig_alloc(void) {
    int ret = 0;
    habtrig_t *habtrig   = NULL;

    habtrig = (habtrig_t *)malloc(sizeof(habtrig_t));

    if (habtrig) {
        habtrig->id = habtrig_cnt;
        habtrig_list[habtrig_cnt++] = habtrig;
    }

    return habtrig;
}

habtrig_t *habtrig_get(const int index) {
    habtrig_t *trig = NULL;

    for (usize i = 0; i < ARRAY_SIZE(habtrig_list); i++) {
        if (index == habtrig_list[i]->index) {
            trig = habtrig_list[i];
            break;
        }
    }

    return trig;
}

void habtrig_free(habtrig_t *trig) {
    int ret = 0;
    char trig_path[128] = IIO_HRTRIG_CONFIGFS_PATH;
    
    strcat(trig_path, trig->name);
    ret = rmdir(trig_path);

    /**
     * TBD - GET RID OF FRAGMENTATION
     * HOW - Linked list implementation.
     */
    habtrig_list[trig->id] = NULL;

    free(trig);
}