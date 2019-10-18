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

/* write_read_i2c first writes write_count bytes from write_buf to the address addr on the
 * i2c bus with the device number dev_num. Then read_count bytes are read into read_buf.
 *
 * The entire exchange is mutex protected with a mutex unique to the device number
 */
int write_read_i2c(int dev_num, unsigned char addr, const void* write_buf,
        size_t write_count, void* read_buf, size_t read_count,
        ssize_t* write_ret, ssize_t* read_ret);

/* check if the field rotator is on a given edge */
char fr_on_edge(char edge);
