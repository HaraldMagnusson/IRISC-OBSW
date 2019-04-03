/* -----------------------------------------------------------------------------
 * Component Name: Img Processing
 * Author(s): 
 * Purpose: Process and store images along with housekeeping data, as well as
 *          sending it to ground.
 * -----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>

#include "global_utils.h"

#include "data_queue.h"
#include "image_handler.h"

#define MODULE_COUNT 2

/* This list controls the order of initialisation */
static const module_init_t init_sequence[MODULE_COUNT] = {
    {"data_queue", &init_data_queue},
    {"image_handler", &init_image_handler}
};

int init_img_processing( void ){

    /* init whatever in this module */
    int ret = init_submodules(init_sequence, MODULE_COUNT);
    if( ret != SUCCESS ){
        return ret;
    }

    return SUCCESS;
}
