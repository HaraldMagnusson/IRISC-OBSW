/* -----------------------------------------------------------------------------
 * Component Name: Kalman Filter
 * Parent Component: Control System
 * Author(s): Anja Möslinger, Harald Magnusson
 * Purpose: Provide the calculations required for the Kalman Filter.
 *
 * -----------------------------------------------------------------------------
 */

#pragma once

int init_kalman_filter(void* args);

int kf_update(telescope_att_t* cur_att);
