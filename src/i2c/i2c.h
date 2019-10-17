/* -----------------------------------------------------------------------------
 * Component Name: I2C
 * Author(s): Harald Magusson
 * Purpose: Provide initilisation and protection for the I2C buses.
 * -----------------------------------------------------------------------------
 */

#pragma once

int init_i2c(void* args);

ssize_t read_i2c(int dev_num, unsigned char addr, void* buf, size_t count);

ssize_t write_i2c(int dev_num, unsigned char addr, const void* buf, size_t count);

/* check if the field rotator is on a given edge */
char fr_on_edge(char edge);
