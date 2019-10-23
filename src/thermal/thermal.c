/* -----------------------------------------------------------------------------
 * Component Name: Thermal
 * Author(s): 
 * Purpose: Monitor the temperatures of the experiment and keep them within
 *          specifications using the heating and cooling systems.
 * -----------------------------------------------------------------------------
 */

#include <pthread.h>

#include "global_utils.h"

//static void* thermal_thread(void* args);
//static void active_m(void);

int init_thermal(void* args){
    //return create_thread("thermal", thermal_thread, 20);
    return SUCCESS;
}
#if 0
static void* thermal_thread(void* args){

    struct timespec wake;
    clock_gettime(CLOCK_MONOTONIC, &wake);

    while(1){
        active_m();

        wake.tv_sec += TEMP_SAMPLE_TIME;
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wake, NULL);

    }

    return NULL;
}

static void active_m(void){

}
#endif
