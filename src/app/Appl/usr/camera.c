#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "camera.h"

/* FIX TO PATH STORAGE */
#include "utils.h"

#define CAMERA_STILL_CMD 0
#define CAMERA_VIDEO_CMD 1

#define MEDIA_PHOTOS   "/photos"
#define MEDIA_VIDEOS   "/videos"

#define STILL_BASENAME "still"
#define STILL_FORMAT   ".jpg"

#define VIDEO_BASENAME "video"
#define VIDEO_FORMAT   ".mkv"

#define VIDEO_COUNTDOWN 3U


int dev_counters[2] = {VIDEO_COUNTDOWN, VIDEO_COUNTDOWN};

typedef enum {
    CAM_STILL,
    CAM_VIDEO,
} media_t;

static void get_output_name(media_t media, const char *cam_tag, char *buffer, usize size) {
    struct timespec tim;

    memset(buffer, 0, size);
    clock_gettime(CLOCK_MONOTONIC, &tim);

    switch (media) {
    case CAM_STILL:
        snprintf(buffer, size, "%s%s/%s-%s-%llu%s", 
            HAB_DATASTORAGE_PATH, MEDIA_PHOTOS, cam_tag, STILL_BASENAME, (unsigned long long)tim.tv_sec, STILL_FORMAT);
        break;
    case CAM_VIDEO:
        snprintf(buffer, size, "%s%s/%s-%s-%llu%s", 
            HAB_DATASTORAGE_PATH, MEDIA_VIDEOS, cam_tag, VIDEO_BASENAME, (unsigned long long)tim.tv_sec, VIDEO_FORMAT);
        break;
    default:
        break;
    }
}

void camera_run(const habdev_t *habdev) {
    u8 op = 0;
    char output_path[128] = {0};
    char action_cmd[256] = {0};
    int curr_idx = 0;
    if (0 == str_compare(habdev->path.dev_name, "imx477_01"))
        curr_idx = 0;
    else
        curr_idx = 1;

    if (NULL != habdev->path.channel[CAMERA_VIDEO_CMD])
        dev_counters[curr_idx]--;

    if (dev_counters[curr_idx] > 0) {
        get_output_name(CAM_STILL, habdev->path.dev_name, output_path, sizeof(output_path));
        op = CAMERA_STILL_CMD;
    } else {
        get_output_name(CAM_VIDEO, habdev->path.dev_name, output_path, sizeof(output_path));
        op = CAMERA_VIDEO_CMD;
        dev_counters[curr_idx] = VIDEO_COUNTDOWN;
    }

    snprintf(action_cmd, sizeof(action_cmd), "%s %s", habdev->path.channel[op], output_path);

    system(action_cmd);
}
