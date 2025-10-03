/**
 * @file bmm350.h
 * @author Aditya Saini and Amruth Nadimpally
 * @brief Library to read data from the BMM350 magnetometer over I2C
 * @version 0.1
 * @date 2025-09-30
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <stdbool.h>

bool BMM350_init();

bool BMM350_read_reg();

bool BMM350_read_multi_reg();

bool BMM350_get_data();

bool BMM350_reset();