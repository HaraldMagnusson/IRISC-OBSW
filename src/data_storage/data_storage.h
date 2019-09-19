/* -----------------------------------------------------------------------------
 * Component Name: Data Storage
 * Author(s): 
 * Purpose: Provide initialisation and an interface for the data storage.
 *
 * -----------------------------------------------------------------------------
 */

#pragma once

/* initialise the data storage component */
int init_data_storage(void* args);

/* save data to external data storage */
void save_data( void );
