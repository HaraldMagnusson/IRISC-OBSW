/* -----------------------------------------------------------------------------
 * Component Name: GPIO
 * Author(s): Harald
 * Purpose: Provide any initilisation and utilities needed for the GPIO.
 *          Pins use BCM numbering.
 * -----------------------------------------------------------------------------
 */

#pragma once

#define IN  0
#define OUT 1

#define LOW  0
#define HIGH 1

int init_gpio( void );

/* enable a pin for I/O usage */
int gpio_export(int pin);

/* disenable a pin for I/O usage */
int gpio_unexport(int pin);

/* set direction of pin (input/output) */
int gpio_direction(int pin, int dir);

/* read value from pin */
int gpio_read(int pin, int* val);

/* write value to pin */
int gpio_write(int pin, int val);