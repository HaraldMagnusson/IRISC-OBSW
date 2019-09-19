/* -----------------------------------------------------------------------------
 * Component Name: Telemetry
 * Author(s): 
 * Purpose: Provide a queue for telemetry messages to be sent to ground and
 *          send them when possible.
 * -----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>

#include "global_utils.h"

#include "downlink.h"
#include "downlink_queue.h"

#define MODULE_COUNT 2

/* This list controls the order of initialisation */
static const module_init_t init_sequence[MODULE_COUNT] = {
    {"downlink", &init_downlink},
    {"downlink_queue", &init_downlink_queue}
};

int init_telemetry(void* args){

    /* init whatever in this module */
    int ret = init_submodules(init_sequence, MODULE_COUNT);
    if( ret != SUCCESS ){
        return ret;
    }

    return SUCCESS;
}
