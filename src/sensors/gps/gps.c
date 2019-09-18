/* -----------------------------------------------------------------------------
 * Component Name: GPS
 * Parent Component: Sensors
 * Author(s): Harald Magnusson
 * Purpose: Store the current gps position of the gondola.
 * -----------------------------------------------------------------------------
 */

#include <pthread.h>

#include "global_utils.h"
#include "sensors.h"
#include "gps.h"

static pthread_mutex_t mutex_gps;
static gps_t gps_local;


int init_gps(void* args){

    gps_local.lat = 0;
    gps_local.lon = 0;
    gps_local.alt = 0;
    gps_local.out_of_date = 1;

    int ret = pthread_mutex_init( &mutex_gps, NULL );
    if( ret ){
        logging(ERROR, "GPS",
                "The initialisation of the gps mutex failed with code %d.\n",
                ret);
        return FAILURE;
    }

    return SUCCESS;
}

void get_gps_local(gps_t* gps){

    pthread_mutex_lock(&mutex_gps);

    gps->lat = gps_local.lat;
    gps->lon = gps_local.lon;
    gps->alt = gps_local.alt;
    gps->out_of_date = gps_local.out_of_date;

    pthread_mutex_unlock(&mutex_gps);
}

void set_gps(gps_t* gps){

    pthread_mutex_lock(&mutex_gps);

    gps_local.lat = gps->lat;
    gps_local.lon = gps->lon;
    gps_local.alt = gps->alt;
    gps_local.out_of_date = 0;

    pthread_mutex_unlock(&mutex_gps);
}

void gps_out_of_date(void){

    pthread_mutex_lock(&mutex_gps);

    gps_local.out_of_date = 1;

    pthread_mutex_unlock(&mutex_gps);
}
