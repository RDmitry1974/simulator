#pragma once

#include "mcu.h"

#define ENABLE_INTERRUPT_ADC { adc1_sta_sampl = pADI_ADC1->STA; adc1_sta_sampl = 0; pADI_ADC1->MSKI |= ADCMSKI_RDY; } // Enable ADC  interrupt source
#define DISABLE_INTERRUPT_ADC { pADI_ADC1->MSKI =0; }                                      // Disable  ADC  interrupt source

void adc_init();
int adc_drv();
int adc_drv_tc();


