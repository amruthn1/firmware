/**
 * @file i2c.c
 * @author Aditya Saini and Amruth Nadimpally
 * @brief I2C library for sending and receiving data
 * @version 0.1
 * @date 2025-09-16
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <stdbool.h>
#include "external/STM32CubeF4/Drivers/CMSIS/Device/ST/STM32F4xx/Include/stm32f407xx.h"

#include "common/phal_F4_F7/i2c/i2c.h"
#include "common/phal_F4_F7/gpio/gpio.h"

GPIOInitConfig_t i2c_gpio_config[] = {
    GPIO_INIT_I2C1_SCL_PB8,
    GPIO_INIT_I2C1_SDA_PB9
};

bool PHAL_initI2C(I2C_TypeDef* i2c) {

    if (i2c == I2C1) {
        //Enable I2C clock with corresponding register
        RCC -> APB1ENR |= RCC_APB1ENR_I2C1EN;
        
        //Configure GPIO pins for I2C
        PHAL_initGPIO(i2c_gpio_config,  sizeof(i2c_gpio_config) / sizeof(GPIOInitConfig_t));

        //Reset I2C 
        I2C1 -> CR1 |= (1<<15); 
        I2C1 -> CR1 &= ~(1<<15);

        //Set clock speed to 45 MHz
        I2C1 -> CR2 |= (45<<0);

        //Configure clock control register
        I2C1 -> CCR = 225<<0;

        //Configure time rise register
        I2C1 -> TRISE = 46;

        //Enable I2C
        I2C1 -> CR1 |= I2C_CR1_PE;

    } else if (i2c == I2C2){
        //Enable I2C clock with corresponding register
        RCC -> APB1ENR |= RCC_APB1ENR_I2C2EN;

        //Configure GPIO pins for I2C
        PHAL_initGPIO(i2c_gpio_config,  sizeof(i2c_gpio_config) / sizeof(GPIOInitConfig_t));

        //Reset I2C 
        I2C2 -> CR1 |= (1<<15); 
        I2C2 -> CR1 &= ~(1<<15);

        //Set clock speed to 45 MHz
        I2C2 -> CR2 |= (45<<0);
 
        //Configure clock control register
        I2C2 -> CCR = 225<<0;

        //Configure time rise register
        I2C2 -> TRISE = 46;

        // Enable I2C
        I2C2 -> CR1 |= I2C_CR1_PE;  
    } 

    return true;
}


bool HAL_I2C_gen_start(I2C_TypeDef* i2c, uint8_t address, uint8_t length, I2CDirection_t mode) {

    // uint32_t timeout = 0; 
    // while ((i2c->SR1 & I2C_ISR_BUSY) && ++timeout < PHAL_I2C_TX_TIMEOUT);
    // if (timeout == PHAL_I2C_TX_TIMEOUT)
    //     return false;
    // timeout = 0;

    // i2c -> CR2 &= 0xF0000000; 
    // if(mode == PHAL_I2C_MODE_RX) { 
        
    // }

    return true; 
    
}

bool PHAL_I2C_read(I2C_TypeDef* i2c, uint8_t* data_a) { 

    I2C1->CR1 &= ~(1<<10);  // clear the ACK bit 
    uint8_t temp = I2C1->SR1 | I2C1->SR2;  // read SR1 and SR2 to clear the ADDR bit.... EV6 condition
    I2C1->CR1 |= (1<<9);  // Stop I2C

    return true;
    
}

bool PHAL_I2C_read_multi(I2C_TypeDef* i2c, uint8_t* data_a, uint8_t size) {
    return true;
}

bool PHAL_I2C_gen_stop(I2C_TypeDef* i2c) {
    return true;
}