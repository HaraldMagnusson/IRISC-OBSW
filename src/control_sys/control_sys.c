/* -----------------------------------------------------------------------------
 * Component Name: Tracking
 * Author(s): 
 * Purpose: Stabilise the telescope. Keep track of the currently highest 
 *          priority target. Provide an interface to update target 
 *          priority list.
 * -----------------------------------------------------------------------------
 */

#include <string.h>

#include "global_utils.h"

#include "stabilization.h"
#include "current_target.h"
#include "gimbal.h"
#include "target_selection.h"
#include "kalman_filter.h"
#include "pid.h"

#define MODULE_COUNT 6

/* This list controls the order of initialisation */
static const module_init_t init_sequence[MODULE_COUNT] = {
    {"tar__selection", &init_target_selection},
    {"current_target", &init_current_target},
    {"stabilization", &init_stabilization},
    {"kalman_filter", &init_kalman_filter},
    {"gimbal", &init_gimbal},
    {"pid", &init_pid}
};

int init_control_sys(void* args){

    /* init whatever in this module */
    return init_submodules(init_sequence, MODULE_COUNT);
}

/* Set the error thresholds for when to start exposing camera */
void set_error_thresholds(double az, double alt_ang){
    set_error_thresholds_local(az, alt_ang);
}
