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
#include <stdint.h>

#include "common/phal_F4_F7/i2c/i2c.h"

#define BMM350_ADDRESS         0x14
#define BMM350_START_ADDR      0x31
#define BMM350_RESET_ADDR      0x7E
#define BMM350_SOFT_RESET_ADDR 0xB6

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
    int16_t temperature;
    uint8_t status[4];
} BMM350_Data;

bool BMM350_init(I2C_TypeDef* i2c) {
    PHAL_initI2C(i2c);
    return true;
}

bool BMM350_read_reg(I2C_TypeDef* i2c, uint8_t reg, uint8_t* val) {
    //Start I2C communication with BMM350
    if (!PHAL_I2C_gen_start(i2c, BMM350_ADDRESS, 1, PHAL_I2C_MODE_TX)) {
        return false;
    }

    //Write register address to read from
    if (!PHAL_I2C_write(i2c, reg)) {
        return false;
    }

    //Restart I2C communication with BMM350 for reading
    if (!PHAL_I2C_gen_start(i2c, BMM350_ADDRESS, 1, PHAL_I2C_MODE_RX)) {
        return false;
    }

    //Read value from register
    if (!PHAL_I2C_read(i2c, val)) {
        return false;
    }

    //End I2C communication with BMM350
    if (!PHAL_I2C_gen_stop(i2c)) {
        return false;
    }

    return true;
}

bool BMM350_read_multi_reg(I2C_TypeDef* i2c, uint8_t reg, uint8_t* buf, uint8_t length) {
    //Start I2C communication
    if (!PHAL_I2C_gen_start(i2c, BMM350_ADDRESS, 1, PHAL_I2C_MODE_TX)) {
        return false;
    }

    //Send register address from where we want to read from
    if (!PHAL_I2C_write(i2c, reg)) {
        return false;
    }

    //Generate start condition, switch to recieve mode
    if (!PHAL_I2C_gen_start(i2c, BMM350_ADDRESS, 1, PHAL_I2C_MODE_RX)) {
        return false;
    }
    
    //Read a number of bytes into the buffer
    if (!PHAL_I2C_read_multi(i2c, buf, length)) {
        return false;
    }

    //End i2c communication
    if (!PHAL_I2C_gen_stop(i2c)) {
        return false;
    }

    return true;
}

bool BMM350_get_data(I2C_TypeDef* i2c, BMM350_Data* data) {
    //Buffer to hold sensor data
    uint8_t buf[8];

    //Read 8 bytes of data starting from the start address
    if (!BMM350_read_multi_reg(i2c, BMM350_START_ADDR, buf, 8)) {
        return false;
    }

    //Map data to struct
    data->x = ((buf[1] << 8) | buf[0]);
    data->y = ((buf[3] << 8) | buf[2]);
    data->z = ((buf[5] << 8) | buf[4]);
    data->temperature = ((buf[7] << 8) | buf[6]);

    return true;
}

bool BMM350_reset(I2C_TypeDef* i2c) {
    //Start I2C communication with BMM350
    if (!PHAL_I2C_gen_start(i2c, BMM350_ADDRESS, 1, PHAL_I2C_MODE_TX)) {
        return false;
    }

    //Write to reset register
    if (!PHAL_I2C_write(i2c, BMM350_RESET_ADDR)) {
        return false;
    }

    //Write to soft reset command
    if (!PHAL_I2C_write(i2c, BMM350_SOFT_RESET_ADDR)) {
        return false;
    }

    //End I2C communication with BMM350
    if (!PHAL_I2C_gen_stop(i2c)) {
        return false;
    }
    return true;
}
