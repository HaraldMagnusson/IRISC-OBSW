/* -----------------------------------------------------------------------------
 * Component Name: Controller
 * Parent Component: Tracking
 * Author(s): Adam Śmiałek
 * Purpose: Stabilise the telescope on the current target.
 *
 * -----------------------------------------------------------------------------
 */

#include <stdio.h>
#include "global_utils.h"
#include "simple_sim.h"
#include "rtwtypes.h"

ExtU_simple_sim_T simple_sim_U;
ExtY_simple_sim_T simple_sim_Y;


int init_stabilization(void){
    return SUCCESS;
}

int stabilization_loop_step(){
    printf("Hello world!\n");
    int ans = 0;
    simple_sim_initialize();

    for (int i = 0; i < 10; i++)
    {
        simple_sim_U.In1 = i;
        simple_sim_step();
        int a = (int)simple_sim_U.In1;
        int b = (int)simple_sim_Y.Out1;
        printf("%d -> %d\n", a, b);
        /* code */
    }

    simple_sim_terminate();
    // ans = simple_sim_step();
    printf("%x", ans);
    return 0;
}
