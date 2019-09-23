/* -----------------------------------------------------------------------------
 * Component Name: Init
 * Author(s): Harald Magnusson
 * Purpose: Called by OS. Initialise the entire system and start threads.
 *
 * -----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <fcntl.h>

#include "camera.h"
#include "command.h"
#include "data_storage.h"
#include "e_link.h"
#include "global_utils.h"
#include "gpio.h"
#include "img_processing.h"
#include "mode.h"
#include "sensors.h"
#include "telemetry.h"
#include "thermal.h"
#include "tracking.h"
#include "watchdog.h"

/* not including init */
#define MODULE_COUNT 13

static int init_func(char* const argv[]);
static int state_machine(void);
static void normal_m(void);
static void reset_m(void);
static void sleep_m(void);
static void wake_up_m(void);

static int ret;
static struct sigaction sa;

static char gyro_wake_flag, rotate_flag, float_flag;
static char rotate_flag_fn[100], float_flag_fn[100];

/* This list controls the order of initialisation */
static const module_init_t init_sequence[MODULE_COUNT] = {
    {"watchdog", &init_watchdog},
    {"mode", &init_mode},
    {"gpio", &init_gpio},
    {"camera", &init_camera},
    {"command", &init_command},
    {"data_storage", &init_data_storage},
    {"e_link", &init_elink},
    {"global_utils", &init_global_utils},
    {"img_processing", &init_img_processing},
    {"sensors", &init_sensors},
    {"telemetry", &init_telemetry},
    {"thermal", &init_thermal},
    {"tracking", &init_tracking}
};

static void sigint_handler(int signum){
    write(STDOUT_FILENO, "\nSIGINT caught\n", 15);

    stop_watchdog();
    write(STDOUT_FILENO, "watchdog stopped\n", 17);

    #ifdef __arm__
        gpio_unexport(GYRO_TRIG_PIN);
    #elif defined(__aarch64__)
        gpio_unexport(GYRO_TRIG_PIN);
    #endif
    write(STDOUT_FILENO, "gpio pin unexported\n", 20);

    pid_t st_pid = get_star_tracker_pid();
    if(st_pid != FAILURE){
        kill(st_pid, SIGTERM);
        write(STDOUT_FILENO, "SIGTERM sent to star tracker\n", 29);
        waitpid(st_pid, NULL, 0);
        write(STDOUT_FILENO, "exiting\n\n", 9);
    }

    _exit(EXIT_SUCCESS);
}

int main(int argc, char* const argv[]){

    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    /* redirect stderr to a log file */
    /* freopen("../test.log", "w", stderr); */

    ret = mlockall(MCL_CURRENT|MCL_FUTURE);
    if( ret != 0 ){
        logging(ERROR, "INIT",
            "init: Failed mlockall. Return value: %d, %s", errno, strerror(errno));
        //return FAILURE;
    }

    if(init_func(argv)){
        return FAILURE;
    }
    /* initialization sequence done */

    strcpy(rotate_flag_fn, get_top_dir());
    strcat(rotate_flag_fn, "output/init_rotate_flag.log");

    strcpy(float_flag_fn, get_top_dir());
    strcat(float_flag_fn, "output/init_float_flag.log");

    int fd = open(rotate_flag_fn, O_RDONLY);
    read(fd, &rotate_flag, 1);
    close(fd);

    fd = open(float_flag_fn, O_RDONLY);
    read(fd, &float_flag, 1);
    close(fd);

    rotate_flag = 0;
    float_flag = 0;

    state_machine();

    return FAILURE;
}

static int init_func(char* const argv[]){

    int count = 0;
    for(int i=0; i<MODULE_COUNT; ++i){
        if(strcmp(init_sequence[i].name, "global_utils") == 0){
            ret = init_sequence[i].init(argv[0]);
        }
        else{
            ret = init_sequence[i].init(NULL);
        }

        if( ret == SUCCESS ){
            logging(INFO, "INIT", "Module \"%s\" initialised successfully.",
                init_sequence[i].name);
        } else if( ret == FAILURE ){
            logging(ERROR, "INIT", "Module \"%s\" FAILED TO INITIALISE, return value: %d",
                init_sequence[i].name, ret);
            ++count;
        } else {
            logging(ERROR, "INIT", "Module \"%s\" FAILED TO INITIALISE, return value: %d, %s",
                init_sequence[i].name, ret, strerror(ret));
            ++count;
        }
    }

    logging(INFO, "INIT",
        "A total of %d modules initialised successfully and %d failed.",
        MODULE_COUNT-count, count);

    if(count != 0){
        return FAILURE;
    }

    return SUCCESS;
}


static int state_machine(void){

    while(1){
        switch(get_mode()){
            case NORMAL:
                normal_m();
                break;

            case SLEEP:
                sleep_m();
                sleep(1);
                break;

            case RESET:
                reset_m();
                break;

            case WAKE_UP:
                wake_up_m();
                break;
        }
    }

    return FAILURE;
}

static void sleep_m(void){

    int fd;

    #if 0
    char ch = fgetc(stdin);

    if(ch == 'r'){
        printf("resetting\n");
        set_mode(RESET);
    }
    #else

    fd = open(float_flag_fn, O_RDONLY);
    read(fd, &float_flag, 1);
    close(fd);

    if(float_flag){
        set_mode(WAKE_UP);
        return;
    }

    gps_t gps;
    get_gps(&gps);

    if(!gps.out_of_date && gps.alt > 5000 && !rotate_flag){
        /* rotate telescope */

        /* set flag */
        rotate_flag = 1;

        /* write flag to storage */
        fd = open(rotate_flag_fn, O_WRONLY);
        write(fd, &rotate_flag, 1);
        close(fd);

    }

    if(!gps.out_of_date && gps.alt > 14000 && !gyro_wake_flag){
        /* wake gyro */

        printf("waking gyroscope\n");
        pthread_mutex_lock(&mutex_cond_gyro);
        pthread_mutex_unlock(&mutex_cond_gyro);
        pthread_cond_signal(&cond_gyro);

        gyro_wake_flag = 1;
    }

    if(!gps.out_of_date && gps.alt > 15000){

        double ang_rate = 0.0;

        /* fetch gondola rotation rate */

        if(ang_rate < ANG_RATE_THRESHOLD){
            set_mode(WAKE_UP);

            /* write flag to storage */
            float_flag = 1;
            fd = open(float_flag_fn, O_WRONLY);
            write(fd, &float_flag, 1);
            close(fd);
        }
    }
    #endif
}

static void normal_m(void){

    char ch = fgetc(stdin);

    if(ch == 'r'){
        set_mode(RESET);
    }
    else if(ch == 's'){
        set_mode(SLEEP);
        printf("entering sleep mode\n");
    }
}

static void reset_m(void){
    printf("entered reset mode\n");
    sleep(0);
    set_mode(WAKE_UP);
}

static void wake_up_m(void){
    printf("entered wake_up mode\n");

    #if 0
    printf("waking star tracker\n");
    pthread_mutex_lock(&mutex_cond_st);
    set_mode(NORMAL);
    pthread_mutex_unlock(&mutex_cond_st);
    pthread_cond_signal(&cond_st);

    sleep(2);

    printf("waking encoder\n");
    pthread_mutex_lock(&mutex_cond_enc);
    pthread_mutex_unlock(&mutex_cond_enc);
    pthread_cond_signal(&cond_enc);

    sleep(2);

    printf("waking gyroscope\n");
    pthread_mutex_lock(&mutex_cond_gyro);
    pthread_mutex_unlock(&mutex_cond_gyro);
    pthread_cond_signal(&cond_gyro);

    #else

    printf("waking star tracker\n");
    pthread_mutex_lock(&mutex_cond_st);
    set_mode(NORMAL);
    pthread_mutex_unlock(&mutex_cond_st);
    pthread_cond_signal(&cond_st);

    sleep(2);

    printf("waking encoder\n");
    pthread_mutex_lock(&mutex_cond_enc);
    pthread_mutex_unlock(&mutex_cond_enc);
    pthread_cond_signal(&cond_enc);

    #endif

    printf("entering normal mode\n");
}