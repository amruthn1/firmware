/**
 * @file i2c.h
 * @author Aditya Saini and Amruth Nadimpally
 * @brief I2C library for sending and receiving data
 * @version 0.1
 * @date 2025-09-16
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef _PHAL_I2C_H
#define _PHAL_I2C_H

#include <stdbool.h>
#include "external/STM32CubeF4/Drivers/CMSIS/Device/ST/STM32F4xx/Include/stm32f407xx.h"

#define PHAL_I2C_TX_TIMEOUT 1000 //(1000U)
#define PHAL_I2C_RX_TIMEOUT 1000 //(1000U)

typedef enum {
    PHAL_I2C_MODE_TX = 0,
    PHAL_I2C_MODE_RX = 1
} I2CDirection_t;

bool PHAL_initI2C(I2C_TypeDef* i2c);

bool PHAL_I2C_gen_start(I2C_TypeDef* i2c, uint8_t address, uint8_t length, I2CDirection_t mode);

bool PHAL_I2C_write(I2C_TypeDef* i2c, uint8_t data);

bool PHAL_I2C_write_multi(I2C_TypeDef* i2c, uint8_t* data, uint8_t size);

bool PHAL_I2C_read(I2C_TypeDef* i2c, uint8_t* data_a);

bool PHAL_I2C_gen_stop(I2C_TypeDef* i2c);

bool PHAL_I2C_read_multi(I2C_TypeDef* i2c, uint8_t* data_a, uint8_t size);

bool PHAL_I2C_gen_stop(I2C_TypeDef* i2c);

#endif