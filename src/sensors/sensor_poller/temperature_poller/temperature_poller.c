/* -----------------------------------------------------------------------------
 * Component Name: Temperature Poller
 * Parent Component: Sensor Poller
 * Author(s): Harald Magnusson
 * Purpose: Poll all thermometers for the current temperature status.
 * -----------------------------------------------------------------------------
 */

#include <pthread.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "global_utils.h"
#include "sensors.h"
#include "temperature_poller.h"
#include "temperature.h"
#include "camera.h"

static void* temp_poller_thread(void* args);
static void active_m(void);
static double get_cpu_temp(void);

static FILE* temp_log;

int init_temperature_poller(void* args){

    char log_fn[100];

    strcpy(log_fn, get_top_dir());
    strcat(log_fn, "output/logs/temp.log");

    temp_log = fopen(log_fn, "a");

    return create_thread("temp_poller", temp_poller_thread, 20);
}

static void* temp_poller_thread(void* args){

    struct timespec wake;
    clock_gettime(CLOCK_MONOTONIC, &wake);

    while(1){
        active_m();

        wake.tv_sec += TEMP_SAMPLE_TIME;
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wake, NULL);

    }

    return NULL;
}

static void active_m(void){

    double temp_nir = get_nir_temp();
    double temp_guiding = get_guiding_temp();
    double temp_gyro = get_gyro_temp();
    double temp_cpu = get_cpu_temp();

    logging_csv(temp_log, "%g,%g,%g,%g",
            temp_nir,
            temp_guiding,
            temp_gyro,
            temp_cpu);
}

static double get_cpu_temp(void){

    FILE* cpu_temp = fopen("/sys/class/thermal/thermal_zone0/temp", "r");

    if(cpu_temp == NULL){
        return -7040;
    }

    char str[5];

    fread(str, 1, 5, cpu_temp);
    fclose(cpu_temp);

    double temp = strtod(str, NULL);
    return temp/1000;
}

