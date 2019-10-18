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
    return init_submodules(init_sequence, MODULE_COUNT);
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

/* set offsets for the azimuth and altitude angle encoders */
int set_enc_offsets(void){
    return set_enc_offsets_l();
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
void set_st_exp(int st_exp){
    set_st_exp_l(st_exp);
}

void set_st_gain(int st_gain){
    set_st_gain_l(st_gain);
}

int get_st_exp(void){
    return get_st_exp_l();
}

/* fetch a single sample from the encoder */
int enc_single_samp(encoder_t* enc){
    return enc_single_samp_l(enc);
}

/* fetch the temperature of the gyroscope */
double get_gyro_temp(void){
    return get_gyro_temp_l();
}

/* update the protected object for the temperature of the gyroscope */
void set_gyro_temp(double temp){
    set_gyro_temp_l(temp);
}

/* fetch the temperatures of the entire system except gyroscope */
void get_temp(temp_t* temp){
    get_temp_l(temp);
}
