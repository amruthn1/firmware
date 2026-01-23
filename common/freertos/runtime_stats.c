#include "runtime_stats.h"

void configureTimer(void)
{
    RUNTIME_TIM_CLK();
    RUNTIME_TIM->PSC = (SystemCoreClock / 1000000) - 1;  // 1 µs tick
    RUNTIME_TIM->ARR = 0xFFFFFFFF;                       
    RUNTIME_TIM->CNT = 0;                                 // reset counter
    RUNTIME_TIM->CR1 |= TIM_CR1_CEN;                      // start timer

}

uint32_t getCounterValue(void)
{
    return TIM2->CNT;
}



