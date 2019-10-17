/* -----------------------------------------------------------------------------
 * Component Name: Sensors
 * Author(s): Harald Magnusson
 * Purpose: Poll sensors and store the latest data. Provide an interface for
 *          other components to read the data.
 * -----------------------------------------------------------------------------
 */

#pragma once

extern pthread_mutex_t mutex_cond_st;
extern pthread_cond_t cond_st;

extern pthread_mutex_t mutex_cond_enc;
extern pthread_cond_t cond_enc;

extern pthread_mutex_t mutex_cond_gyro;
extern pthread_cond_t cond_gyro;

/* The out_of_date flag shows if the available data is the latest (value: 0)
 * or if an error occured in the respective module while updating (value: 1).
 * If an error has occured, the data in the struct is the latest valid data.
 */

typedef struct{
    float lat, lon, alt;
    char out_of_date;
} gps_t;

typedef struct{
    double x, y, z;
    char out_of_date;
} gyro_t;

typedef struct{
    double az, alt_ang;
    char out_of_date;
} encoder_t;

typedef struct{
    double ra, dec, roll;
    char out_of_date, new_data;
} star_tracker_t;

typedef struct{
    double
            pcb_0,
            pcb_1,
            pcb_2,
            ambient,
            motor_az,
            motor_alt,
            motor_roll,
            motor_focus,
            telescope_0,
            telescope_1,
            encoder_0,
            encoder_1,
            nir,
            guiding,
            cpu;
    char out_of_date;
} temp_t;

/* initialise the sensors component */
int init_sensors(void* args);

/* fetch the latest gps data */
void get_gps(gps_t* gps);

/* fetch the latest encoder data */
void get_encoder(encoder_t* encoder);

/* fetch the latest gyro data */
void get_gyro(gyro_t* gyro);

/* set offsets for the azimuth and altitude angle encoders */
int set_enc_offsets(void);

/* fetch the latest star tracker data */
void get_star_tracker(star_tracker_t* st);

/* return the pid for the star tracker child process */
pid_t get_star_tracker_pid(void);

/* set the exposure time (in microseconds) and gain for the star tracker */
void set_st_exp_gain(int st_exp, int st_gain);

/* fetch a single sample from the encoder */
int enc_single_samp(encoder_t* enc);

/* fetch the temperature of the gyroscope */
double get_gyro_temp(void);

/* update the protected object for the temperature of the gyroscope */
void set_gyro_temp(double temp);

/* fetch the temperatures of the entire system except gyroscope */
void get_temp(temp_t* temp);

/* check if the field rotator is on a given edge */
char fr_on_edge(char edge);
