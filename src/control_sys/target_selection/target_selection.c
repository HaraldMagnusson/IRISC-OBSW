/* -----------------------------------------------------------------------------
 * Component Name: Target Selection
 * Parent Component: Control System
 * Author(s): Anja Möslinger, Harald Magnusson
 * Purpose: Keep track of target prioritisation. Update the current target if
 *          a higher priority target is available. Provide an interface to
 *          update target priority list.
 * -----------------------------------------------------------------------------
 */

#include <pthread.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>

#include "global_utils.h"
#include "current_target.h"
#include "target_selection.h"
#include "sensors.h"
#include "camera.h"

static void* sel_track_thread_func(void* arg);

static double d_mod(double val, int mod);
static void angle_calc(double dec, double ha,
        double lat, double* alt, double* az);
static void fetch_time(double* ut_hours, double* j2000);

static int exp_time = 30, sensor_gain = 100;
static pthread_t sel_track_thread_id;

static double az_threshold = 0.01, alt_threshold = 0.01;


int init_target_selection(void* args){

    pthread_create(&sel_track_thread_id, NULL, sel_track_thread_func, NULL);

    return SUCCESS;
}

static void* sel_track_thread_func(void* arg){

    int ret, tar_index;

    char exposing_flag;

    struct timespec wake;

    gps_t gps;
    encoder_t enc;
    telescope_att_t telescope_att;

    double j2000, ut_hours, lst, ha, alt, az, max_prio = 0, gon_az;

    target_prio_t target_prio[19];
    target_t target_list_aa[19], target_err;

    clock_gettime(CLOCK_MONOTONIC, &wake);
    wake.tv_sec++;

    /* selection */
    while(1){

        /* reset camera axis to center */

        /* fetch data: gps, time, gondola attitude(kalman filter + encoder) */
        /* time */
        fetch_time(&ut_hours, &j2000);

        /* gps */
        get_gps(&gps);

        /* encoder */
        get_encoder(&enc);

        /* kahlmann */
        get_telescope_att(&telescope_att);

        /* select target */
        lst = 100.46 + 0.985647 * j2000 + gps.lon + 15 * ut_hours;
        lst = d_mod(lst, 360);

        gon_az = telescope_att.az - enc.az;

        for(int ii=0; ii<19; ++ii){
            /* calculate alt, az, ha for all targets*/
            target_list_aa[ii].ha = lst - 15 * target_list_rd[ii].ra;
            angle_calc(target_list_rd[ii].dec, target_list_aa[ii].ha,
                    gps.lat, &alt, &az);
            target_list_aa[ii].alt = alt;
            target_list_aa[ii].az = az;

            /* position parameter */
            target_prio[ii].pos_param = gon_az - target_list_aa[ii].az;
            target_prio[ii].pos_param = fabs(target_prio[ii].pos_param) < OP_FOV/3 ?
                    OP_FOV/3 - target_prio[ii].pos_param : 0;

            /* exposure parameter */
            target_prio[ii].exp_param = target_prio[ii].exp_num > 10 ?
                    0.1 : exp_prio_list[target_prio[ii].exp_num];

            /* total priority */
            target_prio[ii].tot_prio = target_list_rd[ii].mag *
                    target_prio[ii].pos_param * target_prio[ii].exp_param *
                    target_list_rd[ii].type_prio;
        }

        /* finding maximum priority target */
        for(int ii=0; ii<19; ++ii){
            if( target_prio[ii].tot_prio > max_prio ){
                max_prio = target_prio[ii].tot_prio;
                tar_index = ii;
            }
        }

        #ifdef SELECTION_DEBUG
            logging(DEBUG, "Selection", "Current target: %s", target_list_rd[tar_index].name);
        #endif

        /* selection done */
        exposing_flag = 0;
        /* tracking */
        while(1){

            /* fetch data */
            fetch_time(&ut_hours, &j2000);
            get_gps(&gps);
            get_encoder(&enc);
            get_telescope_att(&telescope_att);

            /* calulate tracking angles */
            lst = 100.46 + 0.985647 * j2000 + gps.lon + 15 * ut_hours;
            lst = d_mod(lst, 360);

            ha = lst - 15 * target_list_rd[tar_index].ra;
            angle_calc(target_list_rd[tar_index].dec, ha, gps.lat, &alt, &az);

            set_tracking_angles(az, alt);

            #ifdef TRACKING_DEBUG
                logging(DEBUG, "Tracking", "Tracking angles: az: %+9.4lf, alt: %+9.4lf",
                        az, alt);
            #endif

            /* abort exposure if target is moving out of operational FoV */
            if(fabs(enc.az) > OP_FOV * 0.45){
                if(exposing_flag){
                    abort_exp_nir("hax.fit");
                    logging(WARN, "Tracking",
                            "Aborted exposure due to telescope leaving operational FoV.");
                }
                break;
            }

            /* camera control */
            target_err.az = az - telescope_att.az;
            target_err.alt = alt - telescope_att.alt;

            if(     !exposing_flag                  &&
                    target_err.az < az_threshold    &&
                    target_err.alt < alt_threshold) {

                /* start exp */
                expose_nir(exp_time, sensor_gain);
                exposing_flag = 1;
                /* set exposing flag */
            }

            if(exposing_flag){
                /* save image */
                ret = save_img_nir("hax.fit");
                if(ret != EXP_NOT_READY){
                    break;
                }
            }
            /* end of camera control */

            /* wait for one sample time */
            wake.tv_nsec += TRACKING_UPDATE_TIME;
            if(wake.tv_nsec >= 1000000000){
                wake.tv_sec++;
                wake.tv_nsec -= 1000000000;
            }
            clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wake, NULL);
        }
    }

    return NULL;
}

/* Modulo opperation on doubles */
static double d_mod(double val, int mod){
    return val - mod*(unsigned long)(val/mod);
}

/* Calculates azimuth and altitude from declination, hour angle, and lattitude */
static void angle_calc(double dec, double ha, double lat, double* alt, double* az){
    dec *= M_PI / 180;
    ha *= M_PI / 180;
    lat *= M_PI / 180;

    *alt = asin(sin(dec)*sin(lat)+cos(dec)*cos(lat)*cos(ha));
    *az = acos((sin(dec) - sin(*alt)*sin(lat)) / (cos(*alt)*cos(lat)));

    if(sin(ha) > 0){
        *az = 2 * M_PI - *az;
    }

    *alt *= 180 / M_PI;
    *az *= 180 / M_PI;
}

/* Fetch the current UTC time in hours with decimals and as julian-2000 date */ 
static void fetch_time(double* ut_hours, double* j2000){
    struct tm date_time;
    time_t epoch_time;

    time(&epoch_time);
    gmtime_r(&epoch_time, &date_time);
    *ut_hours = date_time.tm_hour + date_time.tm_min/60 + date_time.tm_sec/3600;
    *j2000 = 6938.5f + date_time.tm_yday + 1 + *ut_hours/24;
}

/* Set the error thresholds for when to start exposing camera */
void set_error_thresholds_local(double az, double alt_ang){
    az_threshold = az;
    alt_threshold = alt_ang;
}
