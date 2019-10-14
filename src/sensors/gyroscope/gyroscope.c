/* -----------------------------------------------------------------------------
 * Component Name: Gyroscope
 * Parent Component: Sensors
 * Author(s): Harald Magnusson
 * Purpose: Keep track of the angular motion of the telescope.
 *
 * -----------------------------------------------------------------------------
 */

#include <pthread.h>
#include <string.h>

#include "global_utils.h"
#include "sensors.h"
#include "gyroscope.h"

static pthread_mutex_t mutex_gyro = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_gyro_temp = PTHREAD_MUTEX_INITIALIZER;
static gyro_t gyro_local;
static double gyro_temp;

int init_gyroscope(void* args){

    gyro_local.x = 0;
    gyro_local.y = 0;
    gyro_local.z = 0;
    gyro_local.out_of_date = 1;

    return SUCCESS;
}

void get_gyro_local(gyro_t* gyro){

    pthread_mutex_lock(&mutex_gyro);

    *gyro = gyro_local;

    pthread_mutex_unlock(&mutex_gyro);
}

void set_gyro(gyro_t* gyro){

    pthread_mutex_lock(&mutex_gyro);

    gyro_local = *gyro;
    gyro_local.out_of_date = 0;

    pthread_mutex_unlock(&mutex_gyro);
}

/* set the out of date flag on the gyro data */
void gyro_out_of_date(void){

    pthread_mutex_lock(&mutex_gyro);

    gyro_local.out_of_date = 1;

    pthread_mutex_unlock(&mutex_gyro);
}

double get_gyro_temp_l(void){
    pthread_mutex_lock(&mutex_gyro_temp);

    double temp = gyro_temp;

    pthread_mutex_unlock(&mutex_gyro_temp);

    return temp;
}

void set_gyro_temp_l(double temp){

    pthread_mutex_lock(&mutex_gyro_temp);

    gyro_temp = temp;

    pthread_mutex_unlock(&mutex_gyro_temp);
}
