
#include "tr_out.h"
#include "aducm360.h"
#include "mcu.h"
#include <math.h>

//Терморезисторный выход


//-----------------------------------------------------------------------------
void TTROut :: adc_init()
{
    // 00000000  0000    0     001
    //    res    PGA  ADCMOD2 CONT
    pADI_ADC0->MDE = 0x0001;
    // 00000000  0101    0     001
    //    res    PGA  ADCMOD2 CONT
    pADI_ADC1->MDE = 0x0051;  //Gain = 32  ±22.18 mV
    
    //  1      1     0      0      0000    0     1111101
    // CHOP  RAVG2  Res  SINC4EN    AF   NOTCH     SF
    //    pADI_ADC1->FLT = 0xc07d;
   // pADI_ADC1->FLT = 0x9007;
    pADI_ADC1->FLT = 0x807d;
  //  pADI_ADC1->FLT = 0x8E7C;
    
    // 000000000000  0    0       1        1      1       1          00     00   00011 00010
    // Reserved    ADCEN Bipol NegBUFen PosBUFen PosBUFbp NegBUFbp INTREF CurOff AIN3+ AIN2-
 //   pADI_ADC0->CON = 0x000bc062;    
    pADI_ADC0->CON = 0x0003c062;    
    // 000000000000  0    0       0        0      0       0          00     00   00001 00100
    // Reserved    ADCEN Bipol NegBUFen PosBUFen PosBUFbp NegBUFbp INTREF CurOff AIN1+ AIN4-
 //   pADI_ADC1->CON = 0x00080024;
    pADI_ADC1->CON = 0x00000024;
    
    //    1   0     0    00    000     0       0      0000     00
    //  SIMU Res  BOOST  Res  VBias  GND_SW  Gnd20K    Res   EXTBUF
    pADI_ADC1->ADCCFG = 0x8000; //этот регистр управляет обоими АЦП одновременно. Установить одновременную работу 2х АЦП со временем pADI_ADC1->FLT
    NVIC_DisableIRQ(TIMER0_IRQn);
    NVIC_EnableIRQ(ADC1_IRQn);
    NVIC_SetPriority(ADC1_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);  //самое приоритетное пользовательское прерывание
    pADI_ADC1->MSKI |= 1;  //Valid conversion result interrupt
}
//-----------------------------------------------------------------------------
// Обработчик прерывания ADC
/*
void TTROut::vADC_isr()
{
    ADC0_Val = pADI_ADC0->DAT;
    ADC1_Val = pADI_ADC1->DAT;
    xSemaphoreGiveFromISR(xWaitCyclicSemaphore, NULL);
}*/
//-----------------------------------------------------------------------------
void TTROut::port_init()
{
    if(init_param == 0)
        pADI_GP0->GPOUT = KEY2 | KEY3 | KEY4;               //Термо резы 4х
    else
        pADI_GP0->GPOUT = KEY2 | KEY3;                      //Термо резы 3х
    pADI_GP2->GPSET = OFF_I_REG;                            
    pADI_GP2->GPCLR = DAC_ATT;                            
}
//-----------------------------------------------------------------------------
void TTROut::dac_init()
{
    // 00000    0   0    0         0     1          0     1        0      0      00
    //  res    DMA  ON  Norm/NPN  res  buf bypass  UCLK CLR/WORK  12-bit RATE  0-Vref
    //Термо резы 4х
    pADI_DAC->DACCON = 0x0050;
    pADI_DAC->DACDAT = 0x08000000;                 // Output value of 1/2 Vref
  //  pADI_DAC->DACDAT = ((uint32_t)(400. * 0xfff / 1200.)) << 16;//0x08000000;                 // Output value of 400
    dac_write(0x00400000);  // setup 0 V to Vref
    dac_write(0x00380000);  // setup
}
//-----------------------------------------------------------------------------
/*bool TTROut::is_ready_adc_conv()
{
     if ((pADI_ADC1->STA & 0x01))
   // xSemaphoreTake(xWaitCyclicSemaphore, portMAX_DELAY);    //дождаться окончания преобразования АЦП
    {
        ADC0_Val = pADI_ADC0->DAT;
        ADC1_Val = pADI_ADC1->DAT;
        delta = (ADC1_Val - ADC1_Val_Prev);
        ADC1_Val_Prev = ADC1_Val;            
        mVtmp = ADC1_Val * 1200. / 0x10000000;
        if(!((fabs(delta) < 100.) && (fabs(mVtmp) > 0.35)))
            return false;  //ток на уровне шумов. Ну яво.        
        mV = ADC1_Val * 1200. / 0x10000000;       
        mV_2 = ADC0_Val * 1200. / 0x10000000;
        return true;
    }
    return false;
}*/
bool TTROut::is_ready_adc_conv()
{
    //    if ((pADI_ADC1->STA & 0x01))
    if(xSemaphoreTake(xWaitCyclicSemaphore, 1000 / portTICK_RATE_MS) == pdTRUE)    //дождаться окончания преобразования АЦП
    {
    //перенес в прерывание,  а то флаг не сбрасывается
     //   ADC0_Val = pADI_ADC0->DAT;
     //   ADC1_Val = pADI_ADC1->DAT;
        delta = (ADC1_Val - ADC1_Val_Prev);
        ADC1_Val_Prev = ADC1_Val;            
        mVtmp = ADC1_Val * 1200. / 0x10000000;
        if(!((fabs(delta) < 100.) && (fabs(mVtmp) > 0.35)))
            return false;  //ток на уровне шумов. Ну яво.        
        mV = ADC1_Val * 1200. / 0x10000000;       
        mV_2 = ADC0_Val * 1200. / 0x10000000;
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------
//инициализация АЦП, ЦАП, ввода вывода...
void TTROut::init()
{
    mVvalPrev = 600.;
    port_init();
    adc_init();
    dac_init();
}

//-----------------------------------------------------------------------------
void TTROut::cycle_func()
{
    if (is_ready_adc_conv())
    {
        mVE = mV * (value - R_SENS) / R_SENS;       //value в Ом
        //    mV_2 = 690;
        delta_mV = mVE - mV_2;
        //     if(fabs(delta_mV) > 0.05)
        if(fabs(delta_mV) > 0.1)
        {   //подкалибровать
            mVval = mVvalPrev + delta_mV;
            //диапазон выходного напряжения ЦАП 0 - 1200мВ.
            //смещение нижнего конца ~600 мВ
            //Иначе ЦАП войдет в насыщение и из него дооолго выходит.
            if(mVval > 1199.)         
            {
                mVvalPrev = 1199.;
                mVval = 1199.;
            }
            else if(mVval < 0.)
            {
                mVvalPrev = 0.;
                mVval = 0.;
            }
            else
                mVvalPrev = mVval;
            cf = (unsigned int)(mVval * 0xffff0 / 1200.);
            dac_write(0x00300000 + (cf & 0xffff0));
            ++cycle_ctr;
        }
    }
}
