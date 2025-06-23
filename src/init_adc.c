#include "init_adc.h"
#define TIMEOUT 100000


/**
* @brief Check if timeout is reached. This is a function to be used in conjunction with timer_add ()
* @param time Time in cycles to check
* @return 1 if timeout reached 0 if not ( 0 is returned on timeout ) Note : It's not possible to use timer_add () in this
*/
uint8_t timeout(uint32_t time){
    static uint32_t cnt = 0;
    cnt++;
    if (cnt > time){
        return 1;
    }
    return 0;
}

void error(void){
    for(;;){}
}

void init_adc(void)
{
    const int an_shield_pos1 = 2;
    RCC->AHBENR  |= RCC_AHBENR_GPIOBEN;                    // Enable clock for GPIO port B
    RCC->APB2ENR |= RCC_APB2ENR_ADCEN;                     // Enable clock for ADC1 on APB2 bus

    GPIOB->MODER &= ~(0b11 << an_shield_pos1);             // Set PB0 to analog mode by setting MODER0[1:0] // 0b11 für DA-Wandler

    ADC1->CHSELR |= 0b1 << 8;                              // Select ADC channel 8 (PB0) in channel selection register
    ADC1->CFGR1  |= 0b1 << 13;                             // Enable continuous conversion mode (CONT bit)
    ADC1->CFGR1  |= 0b1 << 2;                              // Set scan direction (SCANDIR bit)

    ADC1->SMPR   |= 0b1 << 0;                              // Set sample time 

    if ((ADC1->ISR & (0b1 << 0)) != 0){                    // Check if ADC ready flag is set (bit 0 of ISR)
        ADC1->ISR |= (0b1 << 0);                           // Clear ADC ready flag by writing 1 (write-to-clear)
    }

    ADC1->CR |= 0b1 << 0;    
    
    while ((ADC1->ISR & (0b1 << 0)) == 0){                 // Wait until ADC is ready (ADRDY flag is set)
        if (timeout(TIMEOUT)){                             // Check for timeout during wait
            error();                                       // Handle timeout error
        }
    }// Enable ADC by setting ADEN bit in control register
    


}