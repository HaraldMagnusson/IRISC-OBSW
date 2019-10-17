/* -----------------------------------------------------------------------------
 * Component Name: Temperature
 * Parent Component: Sensors
 * Author(s): Harald Magnusson
 * Purpose: Store and protect the most recent temperature readings.
 *
 * -----------------------------------------------------------------------------
 */

#include <pthread.h>

#include "global_utils.h"
#include "sensors.h"

static temp_t temp_local = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
static pthread_mutex_t mutex_temp = PTHREAD_MUTEX_INITIALIZER;

int init_temperature(void* args){
    return SUCCESS;
}

void get_temp_l(temp_t* temp){

    pthread_mutex_lock(&mutex_temp);

    *temp = temp_local;

    pthread_mutex_unlock(&mutex_temp);
}

void set_temp(temp_t* temp){

    pthread_mutex_lock(&mutex_temp);

    temp_local = *temp;
    temp_local.out_of_date = 0;

    pthread_mutex_unlock(&mutex_temp);
}

/* set the out of date flag on the temp data */
void temp_out_of_date(void){

    pthread_mutex_lock(&mutex_temp);

    temp_local.out_of_date = 1;

    pthread_mutex_unlock(&mutex_temp);
}
