#ifndef INIT_ADC_H
#define INIT_ADC_H

#include "clock_.h"

void init_adc(void);
void error(void);
uint8_t timeout(uint32_t time);

#endif // !INIT_ADC_H
