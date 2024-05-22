#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include "utils.h"
#include "task_main.h"
#include "hab_device.h"

#define TASK_MAIN_SUBPATH "/task_main"
#define TASK_MAIN_LOGFILE "/dev_readout"

static u8 log_format_set;

static stdret_t init_measurement(const ev_glob_t *ev_glob) {
    stdret_t retval  = STD_NOT_OK;
    u32 ch_cnt       = 0;
    habdev_t *habdev = NULL;
    struct stat st           = {0};
    char ch_buffer[128]      = {0};
    char path_buffer[128]    = {0};
    char format_buffer[2048] = {0};
    
    snprintf(path_buffer, sizeof(path_buffer), "%s%s", HAB_DATASTORAGE_PATH, TASK_MAIN_SUBPATH);
    if (stat(path_buffer, &st) == -1)
        retval = (stdret_t)mkdir(path_buffer, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    snprintf(format_buffer, sizeof(format_buffer), "DATA FORMAT: VALUES FROM LEFT TO RIGHT.\nTIMESTAMP ALWAYS IS FIRST\n");
    for (u8 i = 0; i < ev_glob->measured_dev_no; i++) {
        habdev = habdev_get(ev_glob->measured_dev[i]);
        for (u8 ch_num = 0; ch_num < habdev->channel_num; ch_num++) {
            snprintf(ch_buffer, sizeof(ch_buffer), "%d) %s - %s\n", ++ch_cnt, habdev->path.dev_name, habdev->path.channel[ch_num]);
            strcat(format_buffer, ch_buffer);
        }
    }
    strcat(format_buffer, "\n");

    snprintf(path_buffer, sizeof(path_buffer), "%s%s%s", HAB_DATASTORAGE_PATH, TASK_MAIN_SUBPATH, TASK_MAIN_LOGFILE);
    retval = write_file(path_buffer, format_buffer, strlen(format_buffer), MOD_W);

    if (retval == STD_OK)
        log_format_set = 1;

    return retval;
}

void task_runMain(const ev_glob_t *ev_glob) {
    habdev_t *habdev = NULL;
    time_t ts;
    char dev_path[64] = {0};
    char log_buff[128] = {0};
    char path_buff[128] = {0};
    char readout_buff[16] = {0};

    if (0 == log_format_set)
        init_measurement(ev_glob);
    
    ts = time(NULL);
    snprintf (log_buff, sizeof(log_buff), "%ld ", ts);

    for (u8 i = 0; i < ev_glob->measured_dev_no; i++) {
        habdev = habdev_get(ev_glob->measured_dev[i]);
        habdev_getDevPath(habdev, dev_path, sizeof(dev_path));

        for (u8 ch_no = 0; ch_no < habdev->channel_num; ch_no++) {
            snprintf(path_buff, sizeof(path_buff), "%s%s", dev_path, habdev->path.channel[ch_no]);
            (void)read_file(path_buff, readout_buff, sizeof(readout_buff), MOD_R);
            CROP_NEWLINE(readout_buff, strlen(readout_buff));

            strcat(readout_buff, " ");
            strcat(log_buff, readout_buff);
        }
    }
    strcat(log_buff, "\n");

    snprintf(path_buff, sizeof(path_buff), "%s%s%s", HAB_DATASTORAGE_PATH, TASK_MAIN_SUBPATH, TASK_MAIN_LOGFILE);
    (void)write_file(path_buff, log_buff, strlen(log_buff), MOD_A);
}
