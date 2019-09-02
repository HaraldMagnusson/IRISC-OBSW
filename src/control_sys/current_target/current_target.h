/* -----------------------------------------------------------------------------
 * Component Name: Current Target
 * Parent Component: Tracking
 * Author(s): 
 * Purpose: Store the currently highest priority target.
 *
 * -----------------------------------------------------------------------------
 */

#pragma once

/* The out_of_date flag shows if the available data is the latest (value: 0)
 * or if an error occured in the respective module while updating (value: 1).
 * If an error has occured, the data in the struct is the latest valid data.
 */

typedef struct{
    double az, alt;
    char out_of_date;
} telescope_att_t;

/* initialise the current target component */
int init_current_target( void );

/* update the list of targets and priorities */
void set_target_list( void );

/* return the list of targets and priorities */
void set_target_list( void );

void get_telescope_att(telescope_att_t* telescope_att);

void set_telescope_att(telescope_att_t* telescope_att);

void telescope_att_out_of_date(void);

void set_tracking_angles(double az, double alt);

void get_tracking_angles(double* az, double* alt);
