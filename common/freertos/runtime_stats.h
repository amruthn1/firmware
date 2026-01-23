#ifndef RUNTIME_STATS_H
#define RUNTIME_STATS_H

#include <stdint.h>

#if defined(STM32F407xx)
    #include "stm32f4xx.h"
#elif defined(STM32F732xx)
    #include "stm32f7xx.h"
#elif defined(STM32G474xx)
    #include "stm32g4xx.h"
#else
    #error "MCU Arch not supported for runtime stats"
#endif

#if defined(STM32F407xx) || defined(STM32F732xx)
    #define RUNTIME_TIM       TIM2
    #define RUNTIME_TIM_CLK() (RCC->APB1ENR |= RCC_APB1ENR_TIM2EN)
#elif defined(STM32G474xx)
    #define RUNTIME_TIM       TIM2
    #define RUNTIME_TIM_CLK() (RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN)
#else
    #error "MCU Arch not supported for runtime stats"
#endif

void configureTimer(void);
uint32_t getCounterValue(void);

#endif 
