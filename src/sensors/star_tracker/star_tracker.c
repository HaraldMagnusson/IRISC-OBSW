/* -----------------------------------------------------------------------------
 * Component Name: Star Tracker
 * Parent Component: Sensors
 * Author(s): Harald Magnusson
 * Purpose: Keep track of the absolute attitude of the telescope.
 *
 * -----------------------------------------------------------------------------
 */

#include <pthread.h>

#include "global_utils.h"
#include "sensors.h"
#include "star_tracker.h"

static pthread_mutex_t mutex_st;
static star_tracker_t st_local;

int init_star_tracker(void* args){

    st_local.ra = 0;
    st_local.dec = 0;
    st_local.roll = 0;
    st_local.out_of_date = 1;

    int ret = pthread_mutex_init( &mutex_st, NULL );
    if( ret ){
        logging(ERROR, "INIT",
                "The initialisation of the star tracker"
                " mutex failed with code %d.\n", ret);
        return FAILURE;
    }

    return SUCCESS;
}

void get_star_tracker_local(star_tracker_t* st){

    pthread_mutex_lock(&mutex_st);

    st->ra = st_local.ra;
    st->dec = st_local.dec;
    st->roll = st_local.roll;
    st->out_of_date = st_local.out_of_date;

    pthread_mutex_unlock(&mutex_st);
}

void set_star_tracker(star_tracker_t* st){

    pthread_mutex_lock(&mutex_st);

    st_local.ra = st->ra;
    st_local.dec = st->dec;
    st_local.roll = st->roll;
    st_local.out_of_date = 0;

    pthread_mutex_unlock(&mutex_st);
}

void st_out_of_date(void){

    pthread_mutex_lock(&mutex_st);

    st_local.out_of_date = 1;

    pthread_mutex_unlock(&mutex_st);
}
