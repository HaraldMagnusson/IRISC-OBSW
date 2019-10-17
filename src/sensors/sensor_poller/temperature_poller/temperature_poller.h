/* -----------------------------------------------------------------------------
 * Component Name: Temperature Poller
 * Parent Component: Sensor Poller
 * Author(s): Harald Magnusson
 * Purpose: Poll all thermometers for the current temperature status.
 * -----------------------------------------------------------------------------
 */

#pragma once

int init_temperature_poller(void* args);

/* check if the field rotator is on a given edge */
char fr_on_edge_ll(char edge);
