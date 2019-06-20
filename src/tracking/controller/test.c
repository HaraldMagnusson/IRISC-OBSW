#include "simple_sim.h"
#include "rtwtypes.h"
#include <stdio.h>

// struct MODEL {
//     RT_MODEL_simple_sim_T model;

// };

// uint32_T u;
// uint32_T y;
ExtU_simple_sim_T simple_sim_U;
ExtY_simple_sim_T simple_sim_Y;


int main(){
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