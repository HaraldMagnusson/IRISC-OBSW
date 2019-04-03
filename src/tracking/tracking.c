/* -----------------------------------------------------------------------------
 * Component Name: Tracking
 * Author(s): 
 * Purpose: Stabilise the telescope. Keep track of the currently highest 
 *          priority target. Provide an interface to update target 
 *          priority list.
 * -----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>

#include "global_utils.h"

#include "controller.h"
#include "current_target.h"
#include "gimbal.h"
#include "target_selecting.h"

#define MODULE_COUNT 4

/* This list controls the order of initialisation */
static const module_init_t init_sequence[MODULE_COUNT] = {
    {"controller", &init_controller},
    {"current_target", &init_current_target},
    {"gimbal", &init_gimbal},
    {"target_selecting", &init_target_selecting}
};

int init_tracking( void ){

    /* init whatever in this module */
    int ret = init_submodules(init_sequence, MODULE_COUNT);
    if( ret != SUCCESS ){
        return ret;
    }

    return SUCCESS;
}
