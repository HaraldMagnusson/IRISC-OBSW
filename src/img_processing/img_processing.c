/* -----------------------------------------------------------------------------
 * Component Name: Img Processing
 * Author(s): 
 * Purpose: Process and store images along with housekeeping data, as well as
 *          sending it to ground.
 * -----------------------------------------------------------------------------
 */

#include "global_utils.h"

#include "data_queue.h"
#include "image_handler.h"

int init_img_processing( void ){

    /* init whatever in this module */

    int res[2];

    res[0] = init_data_queue();
    res[1] = init_image_handler();

    if( res[0] != SUCCESS ||
        res[1] != SUCCESS
        ){
        return FAILURE;
    }
    return SUCCESS;
}
