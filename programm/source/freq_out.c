
#include "freq_out.h"
#include "aducm360.h"
#include "mcu.h"
#include <math.h>

//Частотный выход


//-----------------------------------------------------------------------------
void TFreqOut :: adc_init()
{
    pADI_ADC0->CON = 0; //повыключать все к чегтям!
    pADI_ADC1->CON = 0;
    NVIC_DisableIRQ(ADC1_IRQn);
    pADI_ADC1->MSKI &= ~1;  //Valid conversion result interrupt disable
    NVIC_EnableIRQ(TIMER0_IRQn);
    NVIC_SetPriority(TIMER0_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);  //самое приоритетное пользовательское прерывание
  //  NVIC_SetPriority(TIMER0_IRQn, 7);  //самое приоритетное пользовательское прерывание
}

//-----------------------------------------------------------------------------
void TFreqOut::port_init()
{
    pADI_GP0->GPOUT = KEY6;                                 
    pADI_GP2->GPCLR = OFF_I_REG;                            
    pADI_GP2->GPCLR = DAC_ATT;                              
}
//-----------------------------------------------------------------------------
void TFreqOut::dac_init()
{
    // 00000    0   0    0         0     1          0     1        0      0      00
    //  res    DMA  ON  Norm/NPN  res  buf bypass  UCLK CLR/WORK  12-bit RATE  0-Vref
    pADI_DAC->DACCON = 0x0050;
    pADI_DAC->DACDAT = 0;                                           // Output value 0V
    dac_write(0x00408000);                                          // setup 0 V to 2 * Vref
    dac_write(0x00300000);                                         //
}

//-----------------------------------------------------------------------------
//#pragma optimize=none
void TFreqOut::reload_timer(float val)
{
    //  000        0      0000  0  00    1      1  0  10
    //Reserved  EVENTEN  EVENT RLD CLK ENABLE  MOD UP PRE
    while(pADI_TM0->STA & 0xC0)__no_operation();
    pADI_TM0->CON = 0x00;           // остановить таймер
 //   pADI_TM0->LD = (unsigned int)((16.0e6 /(256. * 2)) / val);
    pADI_TM0->LD = (unsigned int)((16.0e6 /(16. * 2)) / val);
    while(pADI_TM0->STA & 0xC0)__no_operation();
 //   pADI_TM0->CON = 0x1a;           // запустить таймер /256
    pADI_TM0->CON = 0x19;           // запустить таймер /16
}
//-----------------------------------------------------------------------------
//инициализация АЦП, ЦАП, ввода вывода...
void TFreqOut::init()
{
//    is_port_pin = 0;        
    port_init();
    adc_init();
    dac_init();
    prev_value = value = 50;
    reload_timer(value);
}

//-----------------------------------------------------------------------------
//#pragma optimize=none
void TFreqOut::cycle_func()
{
    if(xSemaphoreTake(xWaitCyclicSemaphore, portMAX_DELAY) == pdTRUE)    //дождаться прерывания таймера
    {
        if(fabs(value - prev_value) > 0.01)
        {
            prev_value = value;
            reload_timer(value);    //перезагрузить таймер новой частотой value в Гц
        }
    }
}
