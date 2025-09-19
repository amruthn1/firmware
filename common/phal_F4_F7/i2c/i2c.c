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

#include "common/phal_F4_F7/i2c/i2c.h"

#include <stdbool.h>

#include "common/phal_F4_F7/gpio/gpio.h"

GPIOInitConfig_t i2c_gpio_config[] = {
    GPIO_INIT_I2C1_SCL_PB8,
    GPIO_INIT_I2C1_SDA_PB9
};

#if defined(STM32F4xx) || defined(STM32F407xx)
    #define IS_F4XX 1
    #define IS_F7XX 0
#elif defined(STM32F7xx) || defined(STM32F732xx)
    #define IS_F4XX 0
    #define IS_F7XX 1
#endif

bool PHAL_initI2C(I2C_TypeDef* i2c) {
    if (i2c == I2C1) {
        //Enable I2C clock with corresponding register
        RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    } else if (i2c == I2C2) {
        //Enable I2C clock with corresponding register
        RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
    }

    //Configure GPIO pins for I2C
    PHAL_initGPIO(i2c_gpio_config, sizeof(i2c_gpio_config) / sizeof(GPIOInitConfig_t));

    //Reset I2C
    i2c->CR1 |= (1 << 15);
    i2c->CR1 &= ~(1 << 15);

    //Configure clock control register and time rise register
    #if IS_F4XX
        //Set clock speed to 45 MHz
        i2c->CR2 |= (45 << 0);
        i2c->CCR = 225 << 0;
        i2c->TRISE = 46;
    #elif IS_F7XX
        i2c->TIMINGR = 0x40912732;
    #endif

        //Enable I2C
        i2c->CR1 |= I2C_CR1_PE;

        return true;
    }

bool HAL_I2C_gen_start(I2C_TypeDef* i2c, uint8_t address, uint8_t length, I2CDirection_t mode) {
    //Reset timeout counter
    uint32_t timeout = 0;

    #if IS_F4XX
        //Wait until I2C is not busy
        while ((i2c->SR2 & I2C_SR2_BUSY) && ++timeout < PHAL_I2C_TX_TIMEOUT);
        if (timeout >= PHAL_I2C_TX_TIMEOUT)
            return false;

        //Configure I2C CR1 register for start 
        i2c->CR1 |= I2C_CR1_START;

        //Wait until start bit is set
        timeout = 0;
        while (!(i2c->SR1 & I2C_SR1_SB) && ++timeout < PHAL_I2C_TX_TIMEOUT);
        if (timeout >= PHAL_I2C_TX_TIMEOUT)
            return false;

        //Send address of slave device
        i2c->DR = (address << 1) | (mode == PHAL_I2C_MODE_RX ? 1 : 0);

        //Wait until address is sent and acknowledged 
        timeout = 0;
        while (!(i2c->SR1 & I2C_SR1_ADDR) && ++timeout < PHAL_I2C_TX_TIMEOUT);
        if (timeout >= PHAL_I2C_TX_TIMEOUT)
            return false;

        //Clear ADDR flag by reading SR1 and SR2
        i2c->SR1;
        i2c->SR2;

    #elif IS_F7XX
        //Wait until I2C is not busy
        while ((i2c->ISR & I2C_ISR_BUSY) && ++timeout < PHAL_I2C_TX_TIMEOUT);
        if (timeout >= PHAL_I2C_TX_TIMEOUT)
            return false;

        //Clear previous configurations
        i2c->CR2 &= ~(I2C_CR2_SADD
                    | I2C_CR2_RD_WRN
                    | I2C_CR2_NBYTES
                    | I2C_CR2_AUTOEND
                    | I2C_CR2_RELOAD
                    | I2C_CR2_START
                    | I2C_CR2_STOP);

        //Configure for start condition
        if (mode == PHAL_I2C_MODE_RX) {
            i2c->CR2 |= I2C_CR2_RD_WRN;
        }
        
        //Set address, number of bytes, and autoend
        i2c->CR2 |= ((uint32_t)(address << 1))
            | (((uint32_t)1) << I2C_CR2_NBYTES_Pos)
            | I2C_CR2_AUTOEND;

        //Start condition
        i2c->CR2 |= I2C_CR2_START;

        //Wait until TXE or RXNE flag is set
        timeout = 0;
        while (!((mode == PHAL_I2C_MODE_RX ? (i2c->ISR & I2C_ISR_RXNE) : (i2c->ISR & I2C_ISR_TXE))) && ++timeout < PHAL_I2C_TX_TIMEOUT);
        if (timeout >= PHAL_I2C_TX_TIMEOUT)
            return false;

    #endif

    return true;
}

bool PHAL_I2C_read(I2C_TypeDef* i2c, uint8_t* data_a) {
    #if IS_F4XX
        //generate start condition for f4, check the startbit
        i2c->CR1 |= I2C_CR1_START;
        while (!(i2c->SR1 & I2C_SR1_SB));

        //clear ACK bit for control register pointer
        I2C1->CR1 &= ~(1 << 10);

        //indicates the slave has acknowleged the the addr by reading SR1 and then SR2
        volatile uint8_t temp = I2C1->SR1 | I2C1->SR2;
        *data_a = I2C1->DR;

        //set ACK bit again
        I2C1->CR1 |= (1 << 9);

    #elif IS_F7XX
        // configure CR2 for a single byte read with AUTOEND to generate stop automatically
        i2c->CR2 = (1 << I2C_CR2_NBYTES_Pos)   // 1 byte to transfer 
                | I2C_CR2_RD_WRN             // read mode
                | I2C_CR2_AUTOEND            //  automatic STOP condition after 1 byte
                | I2C_CR2_START;             // generate a START
        
        // wait until the RXNE flag is set after byte recieved by hardware
        while (!(i2c->ISR & I2C_ISR_RXNE));

        // clear STOPF flag by writing to ICR
        i2c->ICR |= I2C_ICR_STOPCF;
        
    #endif

    return true;
}

bool PHAL_I2C_read_multi(I2C_TypeDef* i2c, uint8_t* data_a, uint8_t size) {
    for (uint8_t i = 0; i < size; i++) {
        if (!PHAL_I2C_read(i2c, &data_a[i])) {
            return false;
        }
    }

    // Stop condition at the end
    #if IS_F4XX
        i2c->CR1 |= I2C_CR1_STOP;
    #elif IS_F7XX
        i2c->CR2 |= I2C_CR2_STOP;
    #endif

    return true;
}

bool PHAL_I2C_write(I2C_TypeDef* i2c, uint8_t data) {
    uint32_t timeout = 0;

    #if ISF4XX 
        //wait for TXE flag
        while (!(i2c->SR1 & I2C_SR1_TXE) && ++timeout < PHAL_I2C_TX_TIMEOUT);
        if (timeout >= PHAL_I2C_TX_TIMEOUT) 
            return false;
        //Write data
        i2c->DR = data;
        //Wait for BTF flag
        timeout = 0;
        while (!(i2c->SR1 & I2C_SR1_BTF) && ++timeout < PHAL_I2C_TX_TIMEOUT);
        if (timeout >= PHAL_I2C_TX_TIMEOUT) 
            return false;
    #elif IS_F7XX
        //wait for TXE flag    
        while (!(i2c->ISR & I2C_ISR_TXIS) && ++timeout < PHAL_I2C_TX_TIMEOUT);
        if (timeout >= PHAL_I2C_TX_TIMEOUT) 
            return false;
        //Write data
        i2c->TXDR = data;
        timeout = 0;
        //Wait for TC flag
        while (!(i2c->ISR & I2C_ISR_TC) && ++timeout < PHAL_I2C_TX_TIMEOUT);
        if (timeout >= PHAL_I2C_TX_TIMEOUT) 
            return false;
    #endif

    return true;
}

bool PHAL_I2C_write_multi(I2C_TypeDef* i2c, uint8_t* data, uint8_t size) {
    for (uint8_t i = 0; i < size; i++) {
        if (!PHAL_I2C_write(i2c, data[i])) {
            return false;
        }
    }
    // Stop 
    #if IS_F4XX
        i2c->CR1 |= I2C_CR1_STOP;
    #elif IS_F7XX
        i2c->CR2 |= I2C_CR2_STOP;
    #endif
    return true;
}

bool PHAL_I2C_gen_stop(I2C_TypeDef* i2c) {
    return true;
}