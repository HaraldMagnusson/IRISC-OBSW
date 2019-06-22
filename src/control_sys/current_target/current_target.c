/* -----------------------------------------------------------------------------
 * Component Name: Current Target
 * Parent Component: Tracking
 * Author(s): 
 * Purpose: Store the currently highest priority target.
 *
 * -----------------------------------------------------------------------------
 */

#include "global_utils.h"
#include "control_sys.h"

int init_current_target( void ){
    filter_current_position = 1;
    tracking_output_angle = 0.5;
    return SUCCESS;
}
