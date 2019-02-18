/* -----------------------------------------------------------------------------
 * Component Name: Tracking
 * Author(s): 
 * Purpose: Stabilise the telescope. Keep track of the currently highest 
 *          priority target. Provide an interface to update target 
 *          priority list.
 * -----------------------------------------------------------------------------
 */

#include "global_utils.h"

#include "controller.h"
#include "current_target.h"
#include "gimbal.h"
#include "target_selecting.h"

int init_tracking( void ){

    /* init whatever in this module */

    int res[4];

    res[0] = init_controller();
    res[1] = init_current_target();
    res[2] = init_gimbal();
    res[3] = init_target_selecting();

    if( res[0] != SUCCESS ||
        res[1] != SUCCESS ||
        res[2] != SUCCESS ||
        res[3] != SUCCESS
        ){
        return FAILURE;
    }
    return SUCCESS;
}
