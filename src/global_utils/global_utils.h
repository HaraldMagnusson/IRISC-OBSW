/* -----------------------------------------------------------------------------
 * Component Name: Global Utils
 * Author(s): Adam Śmiałek, Harald Magnusson
 * Purpose: Define utilities and constants available to all components.
 *
 * -----------------------------------------------------------------------------
 */

#pragma once

/* int function return values */
#define SUCCESS 0
#define FAILURE -1

#define EXP_NOT_READY 1
#define EXP_FAILED 3

/* definitions for socket creation */
#define DOWNLINK_SERVER_PORT 8888
#define DOWNLINK_SERVER_IP   "127.0.0.1"

#define DEBUG   0
#define INFO    1
#define WARN    2
#define ERROR   3
#define CRIT    4

#define GYRO_TRIG_PIN 26

#define GPS_SAMPLE_TIME 4           /* unit: seconds */
#define GYRO_SAMPLE_TIME 10000000    /* unit: nanoseconds */
#define ENCODER_SAMPLE_TIME 10000000 /* unit: nanoseconds */

/* the threshold for the acceptable angular rate of the gondola to start observation phase */
#define GON_ROT_THRESHOLD 1.0 /* unit: degree per second */

/* struct used for initialisation of modules */
typedef int (*init_function)(void* args);
typedef struct {
    const char *name;
    init_function init;
} module_init_t;

/* initialise the global utils component */
int init_global_utils(void* args);

/* top_dir is the absolute path for the top directory
 * of the project evaluated at runtime.
 * get_top_dir returns a pointer to a string containing that directory.
 */
const char* const get_top_dir(void);

int init_submodules(const module_init_t init_sequence[], int module_count);

int logging(int level, char module_name[12],
            const char * format, ... );

/* a call to pthread_create with additional thread attributes,
 * specifically priority
 */
int create_thread(char* comp_name, void *(*thread_func)(void *), int prio);
