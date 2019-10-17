/* -----------------------------------------------------------------------------
 * Component Name: Sensor Poller
 * Parent Component: Sensors
 * Author(s): Harald Magnusson
 * Purpose: Poll sensors and update storage components.
 *
 * -----------------------------------------------------------------------------
 */

#include <stdlib.h>

#include "global_utils.h"

#include "sensors.h"

#include "gps_poller.h"
#include "encoder_poller.h"
#include "gyroscope_poller.h"
#include "star_tracker_poller.h"
#include "temperature_poller.h"

#define MODULE_COUNT 1

/* This list controls the order of initialisation */
static const module_init_t init_sequence[MODULE_COUNT] = {
    //{"encoder_poller", &init_encoder_poller},
    //{"gps_poller", &init_gps_poller},
    //{"gyroscope_poller", &init_gyroscope_poller},
    //{"star_tracker_poller", &init_star_tracker_poller},
    {"temperature_poller", &init_temperature_poller}
};

int init_sensor_poller(void* args){

    /* init whatever in this module */
    return init_submodules(init_sequence, MODULE_COUNT);
}

/* set offsets for the azimuth and altitude angle encoders */
int set_enc_offsets_l(void){
    return set_offsets();
}

/* return the pid for the star tracker child process */
pid_t get_st_pid(void){
    return get_st_pid_local();
}

/* set the exposure time (in microseconds) and gain for the star tracker */
void set_st_exp_gain_l(int st_exp, int st_gain){
    set_st_exp_gain_ll(st_exp, st_gain);
}

int get_st_exp_l(void){
    return get_st_exp_ll();
}

/* fetch a single sample from the encoder */
int enc_single_samp_l(encoder_t* enc){
    return enc_single_samp_ll(enc);
}

/* check if the field rotator is on a given edge */
char fr_on_edge_l(char edge){
    return fr_on_edge_ll(edge);
}
