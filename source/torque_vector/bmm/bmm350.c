/**
 * @file bmm350.c
 * @author Aditya Saini and Amruth Nadimpally
 * @brief Library to read data from the BMM350 magnetometer over I2C
 * @version 0.1
 * @date 2025-09-30
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <stdbool.h>
#include "common/phal_F4_F7/i2c/i2c.h"

bool BMM350_init() {
    PHAL_initI2C(I2C1);
    return true;
}

bool BMM350_read_data() {
    return true;
}

bool BMM350_set_power_mode() {
    return true;
}

bool BMM350_reset() {
    return true;
}


