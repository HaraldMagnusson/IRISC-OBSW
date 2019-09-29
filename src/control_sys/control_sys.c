/* -----------------------------------------------------------------------------
 * Component Name: Tracking
 * Author(s): 
 * Purpose: Stabilise the telescope. Keep track of the currently highest 
 *          priority target. Provide an interface to update target 
 *          priority list.
 * -----------------------------------------------------------------------------
 */

#include "global_utils.h"

#include "stabilization.h"
#include "current_target.h"
#include "gimbal.h"
#include "target_selection.h"

#define MODULE_COUNT 4

/* This list controls the order of initialisation */
static const module_init_t init_sequence[MODULE_COUNT] = {
    {"tar__selection", &init_target_selection},
    {"current_target", &init_current_target},
    {"stabilization", &init_stabilization},
    {"gimbal", &init_gimbal}
};

int init_control_sys(void* args){

    /* init whatever in this module */
    int ret = init_submodules(init_sequence, MODULE_COUNT);
    if( ret != SUCCESS ){
        return ret;
    }

    return SUCCESS;
}

/* Set the error thresholds for when to start exposing camera */
void set_error_thresholds(double az, double alt_ang){
    set_error_thresholds_local(az, alt_ang);
}
