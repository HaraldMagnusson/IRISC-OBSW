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

#define EXP_NOT_READY 1
#define EXP_FAILED 3

#define DEBUG   0
#define INFO    1
#define WARN    2
#define ERROR   3
#define CRIT    4

/* struct used for initialisation of modules */
typedef int (*init_function)(void);
typedef struct {
    const char*   name;
    init_function init;
} module_init_t;

/* initialise the global utils component */
int init_global_utils( void );

int init_submodules( const module_init_t init_sequence[], int module_count);

int logging(int level, char module_name[12],
            const char * format, ... );
