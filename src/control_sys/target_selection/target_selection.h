/* -----------------------------------------------------------------------------
 * Component Name: Target Selection
 * Parent Component: Control System
 * Author(s): Anja Möslinger, Harald Magnusson
 * Purpose: Keep track of target prioritisation. Update the current target if
 *          a higher priority target is available. Provide an interface to 
 *          update target priority list.
 * -----------------------------------------------------------------------------
 */

#pragma once

//TODO: change for actual values
#define OP_FOV 180

typedef struct{
    char name[20];
    double ra, dec, mag;
    int type_prio;
} const_target_t;

typedef struct{
    double pos_param, exp_param, tot_prio;
    int exp_num;
} target_prio_t;

const static const_target_t target_list_rd[19] = {
    {"Andromeda G"          ,0.7            ,41.26666667    ,3.44   ,3},
    {"Pacman N"             ,0.866666667    ,56.61666667    ,7.4    ,4},
    {"Triangulum G"         ,1.55           ,30.65          ,5.72   ,3},
    {"Reflection N"         ,3.483333333    ,31.34833333    ,5.6    ,4},
    {"Star Fish OC"         ,5.466666667    ,35.85          ,7.4    ,1},
    {"Intergalactic Tramp"  ,7.633333333    ,38.86666667    ,9.06   ,2},
    {"Cigar G"              ,9.916666667    ,69.66666667    ,8.41   ,3},
    {"Sunflower G"          ,13.25          ,42.01666667    ,9.3    ,3},
    {"Whirlpool G"          ,13.48333333    ,47.18333333    ,8.4    ,3},
    {"Pinwheel G"           ,14.05          ,54.33333333    ,7.8    ,3},
    {"Hercules GC"          ,16.68333333    ,36.45          ,5.8    ,2},
    {"Eagle N"              ,18.3           ,-12.18333333   ,6      ,4},
    {"Dumbbell N"           ,19.98333333    ,22.71666667    ,7.5    ,4},
    {"Crescent N"           ,20.2           ,38.35          ,7.4    ,4},
    {"Veil N"               ,20.75          ,30.7           ,7      ,4},
    {"Iris N"               ,21.01666667    ,68.16666667    ,6.8    ,4},
    {"Wizard N"             ,22.78333333    ,58.1           ,7.2    ,4},
    {"Bubble N"             ,23.33333333    ,61.2           ,10     ,4},
    {"Open Star Cluster"    ,23.4           ,61.58333333    ,5      ,1}
};

const static int exp_prio_list[11] = {1, 2, 3, 4, 4, 4, 3, 2, 1, 1, 1};

/* initialise the target selection component */
int init_target_selection(void* args);

/* Set the error thresholds for when to start exposing camera */
void set_error_thresholds_local(double az, double alt_ang);

/* Convert ra & dec (ECI) to az & alt (ECEF) */
void rd_to_aa(double ra, double dec, double* az, double* alt);
