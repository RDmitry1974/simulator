
#include "cur_out.h"
#include "aducm360.h"
#include "mcu.h"
#include <math.h>

//Токовый выход


//-----------------------------------------------------------------------------
void TCurOut :: adc_init()
{
    pADI_ADC1->MDE = 0x0031;    //gain 8 +128 mV
   
    //  1      1     0      0      0000    0     1111101
    // CHOP  RAVG2  Res  SINC4EN    AF   NOTCH     SF
    pADI_ADC1->FLT = 0xc07d;

    // 000000000000  1    1         0        0          0       0       00     00    00010 01111
    // Reserved    ADCEN Unipolar NegBUFen PosBUFen PosBUFbp NegBUFbp INTREF CurOff  AIN2+ AGND
    pADI_ADC1->CON = 0x000c004f;
    
    //    0   0     0    00    000     0       0      0000     00
    //  SIMU Res  BOOST  Res  VBias  GND_SW  Gnd20K    Res   EXTBUF
    pADI_ADC1->ADCCFG = 0x0000; 
    NVIC_DisableIRQ(TIMER0_IRQn);
    NVIC_EnableIRQ(ADC1_IRQn);
    NVIC_SetPriority(ADC1_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);  //самое приоритетное пользовательское прерывание
    pADI_ADC1->MSKI |= 1;  //Valid conversion result interrupt
}

//-----------------------------------------------------------------------------
void TCurOut::port_init()
{
    if(init_param == 0)
        pADI_GP0->GPOUT = KEY5;                                 //Ток.
    else
        pADI_GP0->GPOUT = KEY1;                                 //Ток. Запитка от Базиса. 
    pADI_GP2->GPCLR = OFF_I_REG;                            
    pADI_GP2->GPCLR = DAC_ATT;                              //Ток
}
//-----------------------------------------------------------------------------
void TCurOut::dac_init()
{
    // 00000    0   0    0         0     1          0     1        0      0      00
    //  res    DMA  ON  Norm/NPN  res  buf bypass  UCLK CLR/WORK  12-bit RATE  0-Vref
    pADI_DAC->DACCON = 0x0050;
    pADI_DAC->DACDAT = ((unsigned int)(100. * 0xfff / 1200.)) << 16;                 // Output value ~100mV
    dac_write(0x00408000);  // setup 0 V to 2 * Vref
    unsigned int ttt = 100. * 0xffff0 / 2400;
    dac_write(0x00300000 + (ttt & 0xffff0));
}
//-----------------------------------------------------------------------------
bool TCurOut::is_ready_adc_conv()
{
  //  if ((pADI_ADC1->STA & 0x01) != 0)
    if(xSemaphoreTake(xWaitCyclicSemaphore, 1000 / portTICK_RATE_MS) == pdTRUE)    //дождаться окончания преобразования АЦП
    {
   //     ADC1_Val = pADI_ADC1->DAT;         
        mV = ADC1_Val;
        mV = mV * 1200. / 0x10000000;
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------
//инициализация АЦП, ЦАП, ввода вывода...
void TCurOut::init()
{
    mVvalPrev = 100.;
    port_init();
    adc_init();
    dac_init();
}

//-----------------------------------------------------------------------------
void TCurOut::cycle_func()
{
    if (is_ready_adc_conv())
    {
 //       mVE = R_CURR_SENS * value / 1000.;      //value в мкА
        mVE = R_CURR_SENS * value;      //value в мА
       // delta_mV = mVvalPrev - (mVE + mV);
       // if(fabs(delta_mV) > 0.005)
        {   //подкалибровать
            mVval = mVE + mV + 0.766;   //0.766 замеренная поправка. Из-за того, что 2 конец АЦП на земле.
            //диапазон выходного напряжения ЦАП 0 - 2400мВ.
            //смещение нижнего конца ~1200 мВ
            //диапазон выходного напряжения -1000.0 ... +1000.0мВ. 
            //Иначе ЦАП войдет в насыщение и из него дооолго выходит.
            if(mVval > 2399.)         
                mVval = 2399.;
            else if(mVval < 0.)
                mVval = 0.;
            cf = (unsigned int)(mVval * 0xffff0 / 2400);
            dac_write(0x00300000 + (cf & 0xffff0));
      //      ++cycle_ctr;
        }
    }
}
