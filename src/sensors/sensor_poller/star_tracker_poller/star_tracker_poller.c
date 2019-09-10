/* -----------------------------------------------------------------------------
 * Component Name: Star Tracker Poller
 * Parent Component: Sensor Poller
 * Author(s): Niklas Ulfvarson, Harald Magnusson
 * Purpose: Capture images with the guiding camera and perform necessary
 *          calculations to acquire the absolute attitude of the telescope.
 * -----------------------------------------------------------------------------
 */

#include "global_utils.h"
#include <stdlib.h>
#include <stdio.h>

#define TETRAPATH "Tetra/tetra.py"


int init_star_tracker_poller( void ){
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
void irisc_tetra(float st_return[]) {


    FILE *fp = popen("sudo chrt -f 30 python "TETRAPATH, "r");

    for (int i = 0; i < 4; i++) {
        fscanf( fp, "%*s %f", &st_return[i]);
    }

    pclose(fp);
    return;

}
