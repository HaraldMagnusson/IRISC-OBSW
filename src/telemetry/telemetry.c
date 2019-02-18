/* -----------------------------------------------------------------------------
 * Component Name: Telemetry
 * Author(s): 
 * Purpose: Provide a queue for telemetry messages to be sent to ground and
 *          send them when possible.
 * -----------------------------------------------------------------------------
 */

#include "global_utils.h"
#include "downlink.h"
#include "downlink_queue.h"

int init_telemetry( void ){

    /* init whatever in this module */

    int res[2];

    res[0] = init_downlink();
    res[1] = init_downlink_queue();

    if( res[0] != SUCCESS ||
        res[1] != SUCCESS
        ){
        return FAILURE;
    }
    return SUCCESS;
}
