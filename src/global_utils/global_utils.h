/* -----------------------------------------------------------------------------
 * Component Name: Global Utils
 * Author(s): 
 * Purpose: Define utilities and constants available to all components.
 *
 * -----------------------------------------------------------------------------
 */

#pragma once

/* int function return values */
#define SUCCESS 0
#define FAILURE -1

/* definitions for socket creation */
#define DOWNLINK_SERVER_PORT 8888
#define DOWNLINK_SERVER_IP   "127.0.0.1"

/* struct used for initialisation of modules */
typedef int (*init_function)(void);

typedef struct {
    const char *name;
    init_function init;
} module_init_t;

/* initialise the global utils component */
int init_global_utils(void);

int init_submodules(const module_init_t init_sequence[], int module_count);
