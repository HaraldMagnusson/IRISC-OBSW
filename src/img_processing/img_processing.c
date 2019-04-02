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
