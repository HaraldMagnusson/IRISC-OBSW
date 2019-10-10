/* -----------------------------------------------------------------------------
 * Component Name: Img Processing
 * Author(s): 
 * Purpose: Process and store images along with housekeeping data, as well as
 *          sending it to ground.
 * -----------------------------------------------------------------------------
 */

#include <string.h>

#include "global_utils.h"
#include <pthread.h>
#include "data_queue.h"
#include "image_handler.h"
#include "img_processing.h"

#define MODULE_COUNT 2

static int send_st_cmd = 0;

/* This list controls the order of initialisation */
static const module_init_t init_sequence[MODULE_COUNT] = {
    {"data_queue", &init_data_queue},
    {"image_handler", &init_image_handler}
};

int init_img_processing(void* args){

    /* init whatever in this module */
    int ret = init_submodules(init_sequence, MODULE_COUNT);
    if( ret != SUCCESS ){
        return ret;
    }

    return SUCCESS;
}

int queue_image(char *filepath, int type){
    int p;

    if(type==IMAGE_MAIN){
        p = 40;
    } else if(type==IMAGE_STARTRACKER && send_st_cmd){
        p = 30;
        send_st_cmd=0;
    } else {
        p = 50;
    }

    store_data_local(filepath, p, type);

    return SUCCESS;
}

void send_st(void){
    send_st_cmd = 1;

    return;
}
