/* -----------------------------------------------------------------------------
 * Component Name: GPIO
 * Author(s):
 * Purpose: Provide any initilisation and utilities needed for the GPIO.
 * -----------------------------------------------------------------------------
 */

#include <wiringPi.h>

#include "global_utils.h"

int init_gpio( void ){

    wiringPiSetup();

    return SUCCESS;
}
