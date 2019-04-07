/* ------------------------------------------------------------------------------
 * Component Name: Telemetry
 * Author(s): Adam Smialek
 * Purpose: Provide a queue for telemetry messages to be sent to ground and
 *          send them when possible.
 * -----------------------------------------------------------------------------
 */

// todo: delet this
#include <unistd.h>

#include <stdio.h>
#include <pthread.h>

#include "global_utils.h"

#include "downlink.h"
#include "downlink_queue.h"

#define MODULE_COUNT 2

/* This list controls the order of initialisation */
static const module_init_t init_sequence[MODULE_COUNT] = {
    {"downlink", &init_downlink},
    {"downlink_queue", &init_downlink_queue}
};

// todo: For test purposes only:
void *test_telemetry_generator() {
    for (int i = 0; i < 100; ++i) {
        fprintf(stderr, "Queueing data packet nr. %d...", i);
        send_telemetry_local(i, 0);
        fprintf(stderr, " Queued!\n!");
        sleep(1);
    }
}

void *telemetry_sender() {
    while(1) {
        while(!is_empty()) {
            fprintf(stderr, "Sending data packet...");
            char msg[64];
            int num = read_downlink_queue();
            sprintf(msg, "Message nr.%d", num);
            send_data_packet(msg);
            fprintf(stderr, " Sent!\n!");
        }
        fprintf(stderr, "Queue is empty, waiting.\n");
        sleep(3);
    }
}

int init_telemetry( void ){

    /* init whatever in this module */
    int ret = init_submodules(init_sequence, MODULE_COUNT);
    if( ret != SUCCESS ){
        return ret;
    }

    pthread_t generator, sender;
    fprintf(stderr, "> Starting threading.\n");
    pthread_create(&generator, NULL, test_telemetry_generator, NULL);
    pthread_create(&sender, NULL, telemetry_sender, NULL);

    return SUCCESS;
}
