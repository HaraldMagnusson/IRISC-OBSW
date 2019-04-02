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

int init_telemetry( void ){

    /* init whatever in this module */

    int ret;

    for(int i=0; i<MODULE_COUNT; ++i){
        ret = init_sequence[i].init();
        if( ret == SUCCESS ){
            fprintf(stderr, "\tSub module \"%s\" initialised successfully.\n",
                init_sequence[i].name);
        } else if( ret == FAILURE ){
            fprintf(stderr, "\tSub module \"%s\" FAILED TO INITIALISE, return value: %d\n",
                init_sequence[i].name, ret);
            return ret;
        } else {
            fprintf(stderr, "\tSub module \"%s\" FAILED TO INITIALISE, return value: %d, %s\n",
                init_sequence[i].name, ret, strerror(ret));
            return ret;
        }
    }

    return SUCCESS;
}
