/***********************************************************************************************************************
 * event.cpp                                                                                                           *
 ***********************************************************************************************************************
 * DESCRIPTION :                                                                                                       *
 *       APIs for controlling application events (i.e. timer, filesystem, socket connection etc.)                      *
 * PUBLIC FUNCTIONS :                                                                                                  *
 *       ev_t*               event_alloc(void)                                                                         *
 *       stdret_t            ev_setup(habdev_t *habdev)                                                                *
 *                                                                                                                     *
 * AUTHOR :                                                                                                            *
 *       Yahor Yauseyenka    email: yahoryauseyenka@gmail.com                                                          *
 *                                                                                                                     *
 * VERSION                                                                                                             *
 *       0.0.2               last modification: 20-04-2024                                                             *
 *                                                                                                                     *
 * LICENSE                                                                                                             *
 *       GPL                                                                                                           *
 *                                                                                                                     *
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include <stdlib.h>

#include "event.h"
#include "utils.h"
#include "parser.h"
#include "hab_device_types.h"

/***********************************************************************************************************************
 *  PREPROCESSOR DEFINITIONS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * LOCAL TYPEDEFS DECLARATION
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * GLOBAL VARIABLES DECLARATION
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * LOCAL FUNCTION DEFINITION
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * GLOBAL FUNCTION DEFINITION
 **********************************************************************************************************************/
ev_t *event_alloc(void) {
    ev_t *event = NULL;
    uv_handle_t *handle = NULL;

    event = (ev_t *)malloc(sizeof(ev_t));
    handle = (uv_handle_t *)malloc(sizeof(uv_handle_t) + sizeof(habdev_t));

    event->handle = handle;

    return event;
}

stdret_t ev_setup(habdev_t *habdev) {
    stdret_t ret = STD_NOT_OK;
    cfgtoken_t token;
    ev_t *event = habdev->event;
    usize file_pos = 0;

    char file_path[128] = {0};
    char line[64] = {0};
    
    /* Setup a path to event config file */
    ret = create_path(file_path, 2, HAB_EV_CFG_PATH, habdev->path.dev_name);

    while (get_line(file_path, &file_pos, line, sizeof(line)) >= 0) {
        token = parser_parse(line);

        if (CFG_EV_TIMEOUT == token.cfg_type) {
            event->hcfg.tim_ev.tim_to = atoi(token.val);
        } else if (CFG_EV_REPEAT == token.cfg_type) {
            event->hcfg.tim_ev.tim_rep = atoi(token.val);
        } else {
            fprintf(stderr, "ERROR: Unknown event configuration detected. Code: %d\n", token.cfg_type);
            ret = STD_NOT_OK;
        }
    }
    
    return ret;
}

/***********************************************************************************************************************
 * END OF FILE
 **********************************************************************************************************************/
