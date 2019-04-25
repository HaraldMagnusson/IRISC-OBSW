/* -----------------------------------------------------------------------------
 * Component Name: Telemetry
 * Author(s): Adam Smialek
 * Purpose: Provide a queue for telemetry messages to be sent to ground and
 *          send them when possible.
 *
 * -----------------------------------------------------------------------------
 */

/**
 * Module which has a responsibility of initialising downlink stream and
 * adding checksum to the sent info.
 *
 * @TODO Define the data types for the telemetry.
 * @TODO Define priority values for the possible telemetry types.
 */

#include <stdio.h>
#include <pthread.h>

#include "global_utils.h"

#include "downlink.h"
#include "downlink_queue.h"

#define MODULE_COUNT 2

static downlink_node *downlink_queue = NULL;

/* This list controls the order of initialisation */
static const module_init_t init_sequence[MODULE_COUNT] = {
        {"downlink",       &init_downlink},
        {"downlink_queue", &init_downlink_queue}
};

// TODO: For test purposes only!
void *test_telemetry_generator() {
    for (int i = 0; i < 100; ++i) {
        send_telemetry_local(&downlink_queue, i, 0);
    }
    return NULL;
}

/**
 * Infinite loop for sending telemetry via UDP.
 *
 * @return
 *
 * @TODO Move it to `init_telemetry`?
 * @TODO Redo it all!!!
 */
void *telemetry_sender() {
    while (1) {
        char msg[64];
        int num = read_downlink_queue(&downlink_queue);
        sprintf(msg, "Message nr.%d", num);
        send_data_packet(msg);
    }
    return NULL;
}

/**
 *
 * @return
 *
 * @TODO Add threads parameters and all that security stuff.
 */
int init_telemetry(void) {

    /* init whatever in this module */
    int ret = init_submodules(init_sequence, MODULE_COUNT);
    if (ret != SUCCESS) {
        return ret;
    }

    pthread_t generator;
    pthread_t sender;

    pthread_create(&sender, NULL, telemetry_sender, NULL);
    pthread_create(&generator, NULL, test_telemetry_generator, NULL);

    return SUCCESS;
}
