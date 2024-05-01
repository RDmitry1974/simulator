
#include "u_out.h"
#include "aducm360.h"
#include "mcu.h"
#include <math.h>

//Выход напряжения


//-----------------------------------------------------------------------------
void TUOut :: adc_init()
{
    // 000000000000  1    0       1        1      1       1          00     00   00011 00010
    // Reserved    ADCEN Bipol NegBUFen PosBUFen PosBUFbp NegBUFbp INTREF CurOff AIN3+ AIN2-
    pADI_ADC1->CON = 0x000ac062;
    pADI_ADC1->FLT = 0xc07d;
    // 00000000  0000    0     001
    //    res    PGA  ADCMOD2 CONT
    pADI_ADC1->MDE = 0x1;        //gain 1   ±1200 mV
    //    0   0     0    00    000     0       0      0000     00
    //  SIMU Res  BOOST  Res  VBias  GND_SW  Gnd20K    Res   EXTBUF
    pADI_ADC1->ADCCFG = 0x0000;     //
    // 00000000  0000    0     001
    NVIC_DisableIRQ(TIMER0_IRQn);
    NVIC_EnableIRQ(ADC1_IRQn);
    NVIC_SetPriority(ADC1_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);  //самое приоритетное пользовательское прерывание
    pADI_ADC1->MSKI |= 1;  //Valid conversion result interrupt
}

//-----------------------------------------------------------------------------
void TUOut::port_init()
{
    pADI_GP0->GPOUT = KEY3;                                 //
    pADI_GP2->GPSET = OFF_I_REG;                            
    pADI_GP2->GPCLR = DAC_ATT;                              //
}
//-----------------------------------------------------------------------------
void TUOut::dac_init()
{
    // 00000    0   0    0         0     1          0     1        0      0      00
    //  res    DMA  ON  Norm/NPN  res  buf bypass  UCLK CLR/WORK  12-bit RATE  0-Vref
    pADI_DAC->DACCON = 0x0050;
    pADI_DAC->DACDAT = ((uint32_t)(1200. * 0xfff / 1200.)) << 16;                 // Output value ~1200mV
    dac_write(0x00408000);  // setup 0 V to 2 * Vref
    uint32_t ttt = (uint32_t)(1200. * 0xffff0 / (2400.));
    dac_write(0x00300000 + (ttt & 0xffff0));
}
//-----------------------------------------------------------------------------
bool TUOut::is_ready_adc_conv()
{
  //  if ((pADI_ADC1->STA & 0x01) != 0)
    if(xSemaphoreTake(xWaitCyclicSemaphore, 1000 / portTICK_RATE_MS) == pdTRUE)    //дождаться окончания преобразования АЦП
    {
     //   ADC1_Val = pADI_ADC1->DAT;         
        mV = ADC1_Val;
        mV = mV * 1200. / 0x10000000;
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------
//инициализация АЦП, ЦАП, ввода вывода...
void TUOut::init()
{
    mVvalPrev = 1200.;
    port_init();
    adc_init();
    dac_init();
}

//-----------------------------------------------------------------------------
void TUOut::cycle_func()
{
    if (is_ready_adc_conv())
    {
        mVE = value;        //value в мВ
        delta_mV = mVE - mV;
        if(fabs(delta_mV) > 0.005)
        {   //подкалибровать
            mVval = mVvalPrev + delta_mV;
            //диапазон выходного напряжения ЦАП 0 - 2400мВ.
            //смещение нижнего конца ~1200 мВ
            //диапазон выходного напряжения -1000.0 ... +1000.0мВ. 
            //Иначе ЦАП войдет в насыщение и из него дооолго выходит.
            if(mVval > 2399.)         
            {
                mVvalPrev = 2399.;
                mVval = 2399.;
            }
            else if(mVval < 0.)
            {
                mVvalPrev = 0.;
                mVval = 0.;
            }
            else
                mVvalPrev = mVval;
            cf = (unsigned int)(mVval * 0xffff0 / (2400));
            dac_write(0x00300000 + (cf & 0xffff0));
            ++cycle_ctr;
            //      mVE = 15.85 + v_tc_mkv_set / 1000.;
            //      cf = mVE * 0xffff0 / (2400 * K_DIV_DAC);
            /*       mVE = 600. + v_tc_mkv_set / 1000.;
            cf = mVE * 0xffff0 / (1200);
            if(mVvalPrev != cf)
            {
            mVvalPrev = cf;
            dac_write(0x00300000 + (cf & 0xffff0));*/
        }
    }
}
