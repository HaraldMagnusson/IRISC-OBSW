/* -----------------------------------------------------------------------------
 * Component Name: Current Target
 * Parent Component: Control System
 * Author(s): Harald Magnusson
 * Purpose: Store the currently highest priority target.
 *
 * -----------------------------------------------------------------------------
 */

#include <pthread.h>

#include "global_utils.h"
#include "control_sys.h"
#include "current_target.h"
#include "target_selection.h"
#include "telemetry.h"

static pthread_mutex_t mutex_telescope_att, mutex_track_ang;
static telescope_att_t telescope_att_local;
static target_t current_target;

int init_current_target(void* args){

    int ret = pthread_mutex_init( &mutex_telescope_att, NULL );
    if( ret ){
        logging(ERROR, "Cur Target",
                "The initialisation of the telescope attitude"
                "mutex failed with code %d.\n", ret);
        return FAILURE;
    }

    ret = pthread_mutex_init( &mutex_track_ang, NULL );
    if( ret ){
        logging(ERROR, "Cur Target",
                "The initialisation of the tracking angles"
                "mutex failed with code %d.\n", ret);
        return FAILURE;
    }

    telescope_att_local.az = 0;
    telescope_att_local.alt = 0;
    telescope_att_local.out_of_date = 1;

    return SUCCESS;
}

static int freq_count = 0;

void get_telescope_att(telescope_att_t* telescope_att){

    pthread_mutex_lock(&mutex_telescope_att);

    telescope_att->az = telescope_att_local.az;
    telescope_att->alt = telescope_att_local.alt;
    telescope_att->out_of_date = telescope_att_local.out_of_date;

    pthread_mutex_unlock(&mutex_telescope_att);
}

void set_telescope_att(telescope_att_t* telescope_att){

    pthread_mutex_lock(&mutex_telescope_att);

    telescope_att_local.az = telescope_att->az;
    telescope_att_local.alt = telescope_att->alt;
    telescope_att_local.out_of_date = 0;

    if(freq_count++ == 100){
        char buffer[100];
        snprintf(buffer, 100, "%+011.6lf,%+011.6lf", telescope_att->az, telescope_att->alt);
        send_telemetry(buffer, 1, 0, 0);
        freq_count = 0;
    }

    pthread_mutex_unlock(&mutex_telescope_att);
}

void telescope_att_out_of_date(void){

    pthread_mutex_lock(&mutex_telescope_att);

    telescope_att_local.out_of_date = 1;

    pthread_mutex_unlock(&mutex_telescope_att);
}

void get_tracking_angles(double* az, double* alt){

    pthread_mutex_lock(&mutex_telescope_att);

    *az = current_target.az;
    *alt = current_target.alt;

    pthread_mutex_unlock(&mutex_telescope_att);
}

void set_tracking_angles(double az, double alt){

    pthread_mutex_lock(&mutex_telescope_att);

    current_target.az = az;
    current_target.alt = alt;

    pthread_mutex_unlock(&mutex_telescope_att);
}
