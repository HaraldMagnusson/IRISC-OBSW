/* -----------------------------------------------------------------------------
 * Component Name: Sensors
 * Author(s): Harald Magnusson
 * Purpose: Poll sensors and store the latest data. Provide an interface for
 *          other components to read the data.
 * -----------------------------------------------------------------------------
 */

#include <stdlib.h>

#include "global_utils.h"
#include "sensors.h"
#include "encoder.h"
#include "gps.h"
#include "gyroscope.h"
#include "sensor_poller.h"
#include "star_tracker.h"
#include "temperature.h"

#define MODULE_COUNT 6

/* This list controls the order of initialisation */
static const module_init_t init_sequence[MODULE_COUNT] = {
    {"encoder", &init_encoder},
    {"gps", &init_gps},
    {"gyroscope", &init_gyroscope},
    {"star_tracker", &init_star_tracker},
    {"temperature", &init_temperature},
    {"sensor_poller", &init_sensor_poller},
};

int init_sensors(void* args){

    /* init whatever in this module */
    int ret = init_submodules(init_sequence, MODULE_COUNT);
    if( ret != SUCCESS ){
        return ret;
    }

    return SUCCESS;
}

/* fetch the latest gps data */
void get_gps(gps_t* gps){
    get_gps_local(gps);
}

/* fetch the latest encoder data */
void get_encoder(encoder_t* encoder){
    get_encoder_local(encoder);
}

/* fetch the latest gyro data */
void get_gyro(gyro_t* gyro){
    get_gyro_local(gyro);
}

/* fetch the latest star tracker data */
void get_star_tracker(star_tracker_t* st){
    get_star_tracker_local(st);
}

/* return the pid for the star tracker child process */
pid_t get_star_tracker_pid(void){
    return get_st_pid();
}

/* set the exposure time (in microseconds) and gain for the star tracker */
void set_st_exp_gain(int st_exp, int st_gain){
    return set_st_exp_gain_l(st_exp, st_gain);
}
