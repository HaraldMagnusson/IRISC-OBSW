/* -----------------------------------------------------------------------------
 * Component Name: Init
 * Author(s): Harald Magnusson
 * Purpose: Called by OS. Initialise the entire system and start threads.
 *
 * -----------------------------------------------------------------------------
 */

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
#include <math.h>

#include "camera.h"
#include "command.h"
#include "e_link.h"
#include "global_utils.h"
#include "gpio.h"
#include "i2c.h"
#include "img_processing.h"
#include "mode.h"
#include "sensors.h"
#include "telemetry.h"
#include "thermal.h"
#include "control_sys.h"
#include "watchdog.h"

/* not including init */
#define MODULE_COUNT 13

static int init_func(char* const argv[]);
static void check_flags(void);
static int state_machine(void);
static void reset_m(void);
static void sleep_m(void);
static void wake_m(void);

#ifdef SEQ_TEST
static void normal_m(void);
#endif

static int ret;

static char gyro_wake_flag = '0', rotate_flag, float_flag;
static char rotate_flag_fn[100], float_flag_fn[100];
static char stderr_buf[4096];

/* This list controls the order of initialisation */
static const module_init_t init_sequence[MODULE_COUNT] = {
    {"watchdog", &init_watchdog},
    {"mode", &init_mode},
    {"gpio", &init_gpio},
    {"i2c", &init_i2c},
    {"camera", &init_camera},
    {"e_link", &init_elink},
    {"command", &init_command},
    {"global_utils", &init_global_utils},
    {"img_processing", &init_img_processing},
    {"sensors", &init_sensors},
    {"telemetry", &init_telemetry},
    {"thermal", &init_thermal},
    {"control_sys", &init_control_sys}
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

    close_socket();
    write(STDOUT_FILENO, "elink socket closed\n", 20);

    pid_t st_pid = get_star_tracker_pid();
    if(st_pid != FAILURE){
        kill(st_pid, SIGKILL);
        write(STDOUT_FILENO, "SIGKILL sent to star tracker\n", 29);
        waitpid(st_pid, NULL, 0);
        write(STDOUT_FILENO, "exiting\n\n", 9);
    }

    _exit(EXIT_SUCCESS);
}

static void exit_func(void){
    sigint_handler(SIGINT);
}

int main(int argc, char* const argv[]){

    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, NULL);

    atexit(&exit_func);

    /* add buffer to stderr */
    if(setvbuf(stderr, stderr_buf, _IOLBF, 4096)){
        logging(ERROR, "INIT", "Failed to set buffer for stderr");
    }

    /* redirect stderr to a log file */
    /* freopen("../test.log", "w", stderr); */

    ret = mlockall(MCL_CURRENT|MCL_FUTURE);
    if( ret != 0 ){
        logging(ERROR, "INIT",
            "init: Failed mlockall. Return value: %m");
        //return FAILURE;
    }

    if(init_func(argv)){
        stop_watchdog();
        return FAILURE;
    }
    /* initialization sequence done */

    check_flags();

    #ifdef SEQ_TEST
        rotate_flag = '0';
        float_flag = '0';
    #endif

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
            logging(INFO, "INIT",
                    "Module \"%s\" initialised successfully.",
                    init_sequence[i].name);
        } else if( ret == FAILURE ){
            logging(ERROR, "INIT",
                    "Module \"%s\" FAILED TO INITIALISE, return value: %d",
                    init_sequence[i].name, ret);
            ++count;
        } else {
            logging(ERROR, "INIT",
                    "Module \"%s\" FAILED TO INITIALISE, return value: %d, %s",
                    init_sequence[i].name, ret, strerror(ret));
            ++count;
        }
    }

    logging(INFO, "INIT",
        "A total of %d modules initialised successfully and %d failed.",
        MODULE_COUNT-count, count);

    if (count != 0) {
        return FAILURE;
    }

    return SUCCESS;
}

static void check_flags(void){

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
}

static int state_machine(void){

    while(1){
        switch(get_mode()){
            case NORMAL:
                #ifdef SEQ_TEST
                    normal_m();
                #else
                    sleep(1);
                #endif
                break;

            case SLEEP:
                sleep_m();
                sleep(1);
                break;

            case RESET:
                reset_m();
                break;

            case WAKE:
                wake_m();
                break;
        }
    }

    return FAILURE;
}

//TODO: rotate telescope
static void sleep_m(void){

    int fd, ret;

    if(float_flag == '1'){
        set_mode(RESET);
        return;
    }

    gps_t gps;
    get_gps(&gps);

    #ifdef SEQ_DEBUG
        logging(DEBUG, "MODE", "altitude: %f", gps.alt);
    #endif

    if(!gps.out_of_date && gps.alt > 5000 && rotate_flag == '0'){
        /* rotate telescope */
        logging(INFO, "MODE", "rotating out telescope");

        //TODO: rotate telescope
        center_telescope();

        /* set flag */
        rotate_flag = '1';

        /* write flag to storage */
        fd = open(rotate_flag_fn, O_WRONLY);
        write(fd, &rotate_flag, 1);
        close(fd);

    }

    if(!gps.out_of_date && gps.alt > 14000 && gyro_wake_flag == '0'){
        /* wake gyro */

        logging(INFO, "MODE", "waking gyroscope");
        pthread_mutex_lock(&mutex_cond_gyro);
        pthread_cond_signal(&cond_gyro);
        pthread_mutex_unlock(&mutex_cond_gyro);

        gyro_wake_flag = '1';
    }

    if(!gps.out_of_date && gps.alt > 15000){
        /* check gondola rotation rate */

        encoder_t enc;
        do{
            ret = enc_single_samp(&enc);
            usleep(1000);
        } while(ret != SUCCESS);

        double sin_alt_ang = sin(enc.alt_ang * M_PI / 180);
        double cos_alt_ang = cos(enc.alt_ang * M_PI / 180);

        gyro_t gyro;
        get_gyro(&gyro);

        if(gyro.out_of_date){
            return;
        }

        double ang_rate = gyro.y * sin_alt_ang - gyro.x * cos_alt_ang;

        if(fabs(ang_rate) < GON_ROT_THRESHOLD){
            /* write flag to storage */
            float_flag = '1';
            fd = open(float_flag_fn, O_WRONLY);
            write(fd, &float_flag, 1);
            close(fd);

            set_mode(WAKE);
        }
        else{
            logging(INFO,
                    "MODE", "Gondola rotation rate too high to start observation: %lf",
                    ang_rate);
        }
    }
}

#ifdef SEQ_TEST
static void normal_m(void){

    char ch = fgetc(stdin);

    if(ch == 'r'){
        set_mode(RESET);
    }
    else if(ch == 's'){
        set_mode(SLEEP);
    }
}
#endif

static void reset_m(void){
    for(int ii=0; ii<45; ++ii){
        logging(INFO, "MODE", "resetting: %d/%d", ii, 45);
        sleep(1);
    }
    set_mode(WAKE);

    logging(INFO, "MODE", "waking gyroscope");
    pthread_mutex_lock(&mutex_cond_gyro);
    pthread_cond_signal(&cond_gyro);
    pthread_mutex_unlock(&mutex_cond_gyro);
}

static void wake_m(void){

    logging(INFO, "MODE", "waking encoder");
    pthread_mutex_lock(&mutex_cond_enc);
    pthread_cond_signal(&cond_enc);
    pthread_mutex_unlock(&mutex_cond_enc);

    logging(INFO, "MODE", "waking control system");
    pthread_mutex_lock(&mutex_cond_cont_sys);
    pthread_cond_signal(&cond_cont_sys);
    pthread_mutex_unlock(&mutex_cond_cont_sys);

    sleep(2);

    logging(INFO, "MODE", "waking star tracker");
    pthread_mutex_lock(&mutex_cond_st);
    pthread_cond_signal(&cond_st);
    pthread_mutex_unlock(&mutex_cond_st);

    set_mode(NORMAL);
}
