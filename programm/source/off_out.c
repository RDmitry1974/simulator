
#include "off_out.h"
#include "aducm360.h"
#include "mcu.h"
#include <math.h>

//Отключить все выходные ножки


//-----------------------------------------------------------------------------
void TOffOut :: adc_init()
{
    // 000000000000  1    0       0        0      0       0          00     00   00011 00010
    // Reserved    ADCEN Bipol NegBUFen PosBUFen PosBUFbp NegBUFbp INTREF CurOff AIN3+ AIN2-
    pADI_ADC1->CON = 0;                 //выключить АЦП
    NVIC_DisableIRQ(TIMER0_IRQn);
    NVIC_DisableIRQ(ADC1_IRQn);
}

//-----------------------------------------------------------------------------
void TOffOut::port_init()
{
    pADI_GP0->GPOUT = 0;                //все повыключать!                    
    pADI_GP2->GPSET = OFF_I_REG;                            
    pADI_GP2->GPSET = DAC_ATT;                              
}
//-----------------------------------------------------------------------------
void TOffOut::dac_init()
{
    // 00000    0   0    0         0     1          0     1        0      0      00
    //  res    DMA  ON  Norm/NPN  res  buf bypass  UCLK CLR/WORK  12-bit RATE  0-Vref
    pADI_DAC->DACCON = 0;
    dac_write(0x00400000);  // setup 0 V to Vref
    uint32_t ttt = (uint32_t)(0. * 0xffff0 / (1200.));
    dac_write(0x00300000 + (ttt & 0xffff0));
}

//-----------------------------------------------------------------------------
//инициализация АЦП, ЦАП, ввода вывода...
void TOffOut::init()
{
    mVvalPrev = 0.;
    port_init();
    adc_init();
    dac_init();
}

//-----------------------------------------------------------------------------
void TOffOut::cycle_func()
{
    vTaskDelay(1000 / portTICK_RATE_MS);
}
