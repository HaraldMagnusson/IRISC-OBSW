/* -----------------------------------------------------------------------------
 * Component Name: Star Tracker Poller
 * Parent Component: Sensor Poller
 * Author(s): Niklas Ulfvarson, Harald Magnusson
 * Purpose: Capture images with the guiding camera and perform necessary
 *          calculations to acquire the absolute attitude of the telescope.
 * -----------------------------------------------------------------------------
 */

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "global_utils.h"
#include "sensors.h"
#include "star_tracker.h"
#include "camera.h"

#define TETRAPATH "Tetra/tetra.py"
#define ST_WAIT_TIME 100*1000*1000

/* macros used in popen2 */
#define READ 0
#define WRITE 1

static void irisc_tetra(float st_return[]);
static int call_tetra(float st_return[]);
static void* st_poller_thread(void* args);
static pid_t popen2(char* const * command, int *infp, int *outfp);
static int capture_image();

static pthread_mutex_t mutex_st_child;
static pid_t py_pid = -1;
static char st_running = 0;

static int exp_time = 5*1000*1000, gain = 300;

int init_star_tracker_poller(void* args){

    int ret = pthread_mutex_init(&mutex_st_child, NULL);
    if(ret){
        logging(MAIN_LOG, ERROR, "Star Tracker",
                "The initialisation of the star tracker child "
                "mutex failed with code %d.\n", ret);
        return FAILURE;
    }

    return create_thread("star_tracker_poller", st_poller_thread, 23);
}

/*
 * TODO: This system will overwrite old files if the system restarts
 *       (might not be an issue since these files are only for the
 *       image handling queue)
 */
static void* st_poller_thread(void* args){
    sleep(1);

    char st_fn[100], out_fp[100], out_fn[100];
    float st_return[4];

    struct timespec wake;
    wake.tv_nsec = ST_WAIT_TIME;
    wake.tv_sec = 0;

    strcpy(st_fn, get_top_dir());
    strcat(st_fn, "output/guiding/star_tracker/st_img.fit");

    strcpy(out_fp, get_top_dir());
    strcat(out_fp, "output/guiding/");

    for(int ii=0; 1; ++ii){

        do{
            /* capture image */
            if(capture_image(st_fn)){
                break;
            }

            /* star tracker calculations */
            if(call_tetra(st_return)){
                st_out_of_date();
                break;
            }

            /* move image to img queue dir */
            snprintf(out_fn, 100, "%s%4d.fit", out_fp, ii);
            rename(st_fn, out_fn);

            /* queue up image */

        } while(0);

        clock_nanosleep(CLOCK_MONOTONIC, 0, &wake, NULL);
    }
    return NULL;
}

/*
 * TODO: error handling from camera
 */
static int capture_image(char* fn){

    int ret;

    ret = expose_guiding(exp_time, gain);
    if(ret != SUCCESS){
        return ret;
    }

    usleep(0.95 * exp_time);

    do{
        usleep(1);
        ret = save_img_guiding(fn);
    } while(ret == EXP_NOT_READY);

    return ret;
}

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

        logging(MAIN_LOG, DEBUG, "Star Tracker",
                "Star tracker sample time: %ld.%09ld s",
                diff.tv_sec, diff.tv_nsec);
        logging(MAIN_LOG, DEBUG, "Star Tracker", "Output: %f, %f, %f, %f",
                st_return[0], st_return[1], st_return[2], st_return[3]);
    #else
        irisc_tetra(st_return);
    #endif

    if(st_return[3] == 0){
        logging(MAIN_LOG, WARN, "Star Tracker", "FoV = 0, lost in space failed");
        return FAILURE;
    }

    st.ra = st_return[0];
    st.dec = st_return[1];
    st.roll = st_return[2];

    set_star_tracker(&st);

    return SUCCESS;
}

/*
 * Purpose: This is the interface to run the Tetra python program from the
 *          main OBSW.
 * Usage: A float array of length 4 i passed, and values from the star tracker
 *          is returned in this array in order:
 *          0: RA
 *          1: Dec
 *          2: Roll
 *          3: FoV
 *
 *          The name of the parameter is printed by tetra, but then ignored.
 *          If no attitude could be calculated all of these will be 0. This
 *          can obviously not happen if an attitude is calculated, as FoV will
 *          always have a positive non-zero value.
 */
static void irisc_tetra(float st_return[]) {

    char* cmd[6] = {
        "chrt",
        "-f",
        "30",
        "python",
        TETRAPATH,
        NULL
    };

    int outfp = -1;

    pthread_mutex_lock(&mutex_st_child);
    st_running = 1;
    py_pid = popen2(cmd, NULL, &outfp);
    waitpid(py_pid, NULL, 0);
    st_running = 0;

    char buffer[100];
    int cntr = 0, ii;
    for(ii = 0; cntr < 4 && ii < 100; ++ii){
        read(outfp, &buffer[ii], 1);
        if(buffer[ii] == '\n'){
            cntr++;
        }
    }
    buffer[ii] = '\0';

    sscanf(buffer, "%*s %f\n" "%*s %f\n" "%*s %f\n" "%*s %f\n",
            &st_return[0], &st_return[1], &st_return[2], &st_return[3]);

    close(outfp);
    return;
}

static pid_t popen2(char* const * command, int *infp, int *outfp){
    int p_stdin[2], p_stdout[2];
    pid_t pid;

    if (pipe(p_stdin) != 0 || pipe(p_stdout) != 0)
        return -1;

    pid = fork();

    if (pid < 0)
        return pid;
    else if (pid == 0)
    {
        close(p_stdin[WRITE]);
        dup2(p_stdin[READ], READ);
        close(p_stdout[READ]);
        dup2(p_stdout[WRITE], WRITE);

        execvp(*command, command);
        perror("execvp");
        exit(1);
    }

    pthread_mutex_unlock(&mutex_st_child);
    if (infp == NULL)
        close(p_stdin[WRITE]);
    else
        *infp = p_stdin[WRITE];

    if (outfp == NULL)
        close(p_stdout[READ]);
    else
        *outfp = p_stdout[READ];

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
void set_st_exp_gain_ll(int st_exp, int st_gain){
    exp_time = st_exp;
    gain = st_gain;
}
