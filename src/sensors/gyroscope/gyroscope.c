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

static pthread_mutex_t mutex_gyro;
static gyro_t gyro_local;

int init_gyroscope( void ){

    gyro_local.x = 0;
    gyro_local.y = 0;
    gyro_local.z = 0;
    gyro_local.out_of_date = 1;


    int ret = pthread_mutex_init(&mutex_gyro, NULL);
    if( ret ){
        logging(ERROR, "Gyro",
                "The initialisation of the gyro mutex failed: %d, (%s)",
                ret, strerror(ret));
        return ret;
    }

    return SUCCESS;
}

void get_gyro_local(gyro_t* gyro){

    pthread_mutex_lock(&mutex_gyro);

    gyro->x = gyro_local.x;
    gyro->y = gyro_local.y;
    gyro->z = gyro_local.z;
    gyro->out_of_date = gyro_local.out_of_date;

    pthread_mutex_unlock(&mutex_gyro);
}

void set_gyro(gyro_t* gyro){

    pthread_mutex_lock(&mutex_gyro);

    gyro_local.x = gyro->x;
    gyro_local.y = gyro->y;
    gyro_local.z = gyro->z;
    gyro_local.out_of_date = 0;

    pthread_mutex_unlock(&mutex_gyro);
}

/* set the out of date flag on the gyro data */
void gyro_out_of_date(void){

    pthread_mutex_lock(&mutex_gyro);

    gyro_local.out_of_date = 1;

    pthread_mutex_unlock(&mutex_gyro);
}
