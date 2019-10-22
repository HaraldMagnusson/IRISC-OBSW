/* -----------------------------------------------------------------------------
 * Component Name: Star Tracker Poller
 * Parent Component: Sensor Poller
 * Author(s): Niklas Ulfvarson, Harald Magnusson
 * Purpose: Capture images with the guiding camera and perform necessary
 *          calculations to acquire the absolute attitude of the telescope.
 * -----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <math.h>

#include "global_utils.h"
#include "sensors.h"
#include "star_tracker.h"
#include "camera.h"
#include "mode.h"
#include "img_processing.h"
#include "current_target.h"

#define ST_WAIT_TIME 10*1000*1000

/* macros used in popen2 */
#define READ 0
#define WRITE 1

static void irisc_tetra(float st_return[]);
static int call_tetra(float st_return[]);
static void* st_poller_thread(void* args);
static pid_t popen2(char* const * command, int *infp, int *outfp);
static void active_m(void);

#ifndef ST_TEST
    static int capture_image();
#endif

pthread_mutex_t mutex_cond_st = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_st = PTHREAD_COND_INITIALIZER;

static pid_t py_pid = -1;
static char st_running = 0;

/* exposure time in seconds */
static int exp_time = 2, gain = 300;

/* filenames for images */
static char st_fn[100], out_fp[100];
static float st_return[4];
static FILE* star_tracker_log;

#ifndef ST_TEST
    static char out_fn[100];
    static int img_cntr = 0;
#endif

static struct timespec wake;

int init_star_tracker_poller(void* args){

    /* set up log file */
    char log_fn[100];

    strcpy(log_fn, get_top_dir());
    strcat(log_fn, "output/logs/star_tracker.log");

    star_tracker_log = fopen(log_fn, "a");
    if(star_tracker_log == NULL){
        logging(ERROR, "Star Tracker",
                "Failed to open star tracker log file, %m");
        return errno;
    }

    /* star tracker setup */
    wake.tv_nsec = ST_WAIT_TIME;
    wake.tv_sec = 0;

    strcpy(st_fn, get_top_dir());
    strcat(st_fn, "output/guiding/star_tracker/st_img.fit");
    /* strcat(st_fn, "output/guiding/tmp/st_img.fit"); */

    strcpy(out_fp, get_top_dir());
    strcat(out_fp, "output/compression/");

    return create_thread("st_poller", st_poller_thread, 23);
}

static void* st_poller_thread(void* args){

    pthread_mutex_lock(&mutex_cond_st);

    while(1){

        pthread_cond_wait(&cond_st, &mutex_cond_st);

        while(get_mode() != RESET){
            active_m();

            clock_nanosleep(CLOCK_MONOTONIC, 0, &wake, NULL);
        }
    }

    return NULL;
}

static void active_m(void){

    #ifndef ST_TEST
        /* capture image */
        if(capture_image(st_fn)){
            st_out_of_date();
            return;
        }
    #endif

    /* star tracker calculations */
    if(call_tetra(st_return)){
        st_out_of_date();
        return;
    }

    #ifndef ST_TEST
        /* move image to img queue dir */
        snprintf(out_fn, 100, "%sst%04d.fit", out_fp, img_cntr++);
        rename(st_fn, out_fn);

        queue_image(out_fn, IMAGE_STARTRACKER);
    #endif
}

#ifndef ST_TEST
static int capture_image(char* fn){

    int ret;

    ret = expose_guiding(exp_time * 1000000, gain);
    if(ret != SUCCESS){
        return ret;
    }

    usleep(0.95 * exp_time);

    do{
        usleep(10000);
        ret = save_img_guiding(fn);
    } while(ret == EXP_NOT_READY);

    return ret;
}
#endif

static int call_tetra(float st_return[]){

    star_tracker_t st;

    #ifdef ST_DEBUG
        struct timespec samp_0, samp, diff;
    #endif

    #ifdef ST_DEBUG
        clock_gettime(CLOCK_MONOTONIC, &samp_0);
        irisc_tetra(st_return);
        clock_gettime(CLOCK_MONOTONIC, &samp);

        diff.tv_sec = samp.tv_sec - samp_0.tv_sec;
        diff.tv_nsec = samp.tv_nsec - samp_0.tv_nsec;
        if(diff.tv_nsec < 0){
            diff.tv_sec--;
            diff.tv_nsec += 1000000000;
        }

        logging(DEBUG, "Star Tracker",
                "Star tracker sample time: %ld.%09ld s",
                diff.tv_sec, diff.tv_nsec);
        logging(DEBUG, "Star Tracker", "Output: %f, %f, %f, %f",
                st_return[0], st_return[1], st_return[2], st_return[3]);
    #else
        irisc_tetra(st_return);
    #endif

    if(fabs(st_return[3]) < 0.001){
        logging(WARN, "Star Tracker", "FoV = 0, lost in space failed");
        return FAILURE;
    }

    st.ra = st_return[0];
    st.dec = st_return[1];
    st.roll = st_return[2];

    logging_csv(star_tracker_log, "%010.6f,%010.7f,%010.6f",
            st.ra, st.dec, st.roll);

    set_star_tracker(&st);

    return SUCCESS;
}

/*
 * Purpose: This is the interface to run the Astrometry.net program from the
 *          main OBSW.
 * Usage: A float array of length 4 i passed, and values from the star tracker
 *          is returned in this array in order:
 *          0: RA
 *          1: Dec
 *          2: Roll
 *          3: FoV
 *
 *          The name of the parameter is printed by the star tracker, but then ignored.
 *          If no attitude could be calculated all of these will be 0. This
 *          can obviously not happen if an attitude is calculated, as FoV will
 *          always have a positive non-zero value.
 */
static void irisc_tetra(float st_return[]) {

    char st_img_path[100];

    /*
    char exe_path[100];
    strcpy(exe_path, get_top_dir());
    strcat(exe_path,"usr/local/astrometry/bin/solve-field");
    */
    strcpy(st_img_path, get_top_dir());
    strcat(st_img_path,"output/guiding/star_tracker/st_img.fit");

    /* lisFlag, 1 if we are completely lost in space. 0 for only slightly. */
    static int lisFlag = 1;
    static char oldRa[4] = "0";
    static char oldDec[4] = "0";
    char* stRad = "15";
    
    char* cmd[10] = {
        "chrt",
        "-f",
        "23",
        "/usr/local/astrometry/bin/solve-field",
        "-pOo",
        "none",
        "--scale-low",
        "1",
        st_img_path,
        NULL
    };

    char* raDecFlags[7] = {
        "--ra",
        oldRa,
        "--dec",
        oldDec,
        "--radius",
        stRad,
        NULL 
    };

    char* raDecCmd[16];

    int out_fd = -1;

    st_running = 1;

    if (!lisFlag) {
        strcpy((char*)raDecCmd, (char*)cmd);
        strcat((char*)raDecCmd, (char*)raDecFlags);

        py_pid = popen2(raDecCmd, NULL, &out_fd);
    } else {
        py_pid = popen2(cmd, NULL, &out_fd);
        //lisFlag = 1;
    }
    waitpid(py_pid, NULL, 0);
    st_running = 0;

    FILE* oFPtr = fdopen(out_fd, "r");

    char buffer[200];

    while (1) {
        fscanf(oFPtr, "%s", buffer);
        if (!strcmp(buffer, "IRISC")) {
            break;
        }
    }

    for (int i = 0; i < 4; i++) {
        fscanf( oFPtr, "%*s %f", &st_return[i]);
    }

    if(fabs(st_return[3]) < 0.001){
        //lisFlag = 0;
    } else {
        sprintf(oldRa, "%d", (int)st_return[1]);
        sprintf(oldDec, "%d", (int)st_return[2]);
    }
    fclose(oFPtr);
    return;
}

static pid_t popen2(char* const * command, int* in_fd, int* out_fd){
    int p_stdin[2], p_stdout[2];
    pid_t pid;

    if(pipe(p_stdin) != 0 || pipe(p_stdout) != 0){
        return FAILURE;
    }

    pid = fork();

    if(pid < 0){
        return pid;
    }
    else if(pid == 0){
        close(p_stdin[WRITE]);
        dup2(p_stdin[READ], READ);
        close(p_stdout[READ]);
        dup2(p_stdout[WRITE], WRITE);

        execvp(*command, command);
        perror("execvp");
        exit(1);
    }

    if(in_fd == NULL){
        close(p_stdin[WRITE]);
    }
    else{
        *in_fd = p_stdin[WRITE];
    }

    if(out_fd == NULL){
        close(p_stdout[READ]);
    }
    else{
        *out_fd = p_stdout[READ];
    }

    return pid;
}

/* return the pid for the star tracker child process */
pid_t get_st_pid_local(void){
    if(st_running){
        return py_pid;
    }
    return FAILURE;
}
/* set the exposure time (in microseconds) and gain for the star tracker */
void set_st_exp_ll(int st_exp){
    exp_time = st_exp;
}
void set_st_gain_ll(int st_gain){
    gain = st_gain;
}

int get_st_exp_ll(void){
    return exp_time;
}
