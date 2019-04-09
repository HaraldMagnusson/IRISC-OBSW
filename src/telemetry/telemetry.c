/* ------------------------------------------------------------------------------
 * Component Name: Telemetry
 * Author(s): Adam Smialek
 * Purpose: Provide a queue for telemetry messages to be sent to ground and
 *          send them when possible.
 * -----------------------------------------------------------------------------
 */

/**
 * Module which has a responsibility of initialising downlink stream and
 * adding checksum to the
 *
 *
 */

// todo: delet this
#include <unistd.h>
#include <time.h>

#include <stdio.h>
#include <pthread.h>

#include "global_utils.h"

#include "downlink.h"
#include "downlink_queue.h"

#define MODULE_COUNT 2

static downlink_node* downlink_queue = NULL;

/* This list controls the order of initialisation */
static const module_init_t init_sequence[MODULE_COUNT] = {
    {"downlink", &init_downlink},
    {"downlink_queue", &init_downlink_queue}
};

// todo: For test purposes only!
void* test_telemetry_generator() {
    for (int i = 0; i < 100; ++i) {
        send_telemetry_local( &downlink_queue, i, 0 );
        sleep(1);
    }

    return NULL;
}

/**
 * Infinite loop for sending telemetry via UDP.
 *
 * @return
 *
 * @todo Move it to `init_telemetry`?
 * @todo Redo it all!!!
 */
void* telemetry_sender() {
    while(1) {
        while(!is_empty(&downlink_queue)) {
            char msg[64];
            int num = read_downlink_queue(&downlink_queue);
            sprintf(msg, "Message nr.%d", num);
            send_data_packet(msg);
        }
        sleep(3);
    }
    return NULL;
}


/**
 *
 * @return
 *
 * @todo Add threads parameters and all that security stuff.
 */
int init_telemetry( void ){

    /* init whatever in this module */
    int ret = init_submodules(init_sequence, MODULE_COUNT);
    if( ret != SUCCESS ){
        return ret;
    }

    pthread_t generator;
    pthread_t sender;

    pthread_create(&generator, NULL, test_telemetry_generator, NULL);
    pthread_create(&sender, NULL, telemetry_sender, NULL);

    return SUCCESS;
}
