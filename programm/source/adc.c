
#include "adc.h"
#include <math.h>

int32_t ADC0_Val   = 0;
int32_t ADC0_Val_Prev   = 0;
//int32_t ADC0_Val_2 = 0;

int32_t ADC1_Val   = 0;
int32_t ADC1_Val_Prev   = 0;
//int32_t ADC1_Val_2 = 0;

int conv = -1;


//-----------------------------------------------------------------------------
void adc_init()
{
#ifdef TERMO_RES        
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
    
    
    //   00        0    00    000     0       0      0000     00
    // Reserved  BOOST  Res  VBias  GND_SW  Gnd20K    Res   EXTBUF
 //   pADI_ADC0->ADCCFG = 0x0000;
        //       pADI_ADC0->FLT = 0x9003;
 //   pADI_ADC0->FLT = 0x9007;
    
#endif

#ifdef TERMO_CAU        
    // 000000000000  1    0       0        0      0       0          00     00   00011 00010
    // Reserved    ADCEN Bipol NegBUFen PosBUFen PosBUFbp NegBUFbp INTREF CurOff AIN3+ AIN2-
    pADI_ADC1->CON = 0x00080062;
    pADI_ADC1->FLT = 0xc07d;
  //  pADI_ADC1->MDE = 0x0049;        //gain 32 ±37.5 mV
    pADI_ADC1->MDE = 0x0019;        //gain 2 ± mV
    pADI_ADC1->ADCCFG = 0x0000;     //
#endif

#ifdef CURRENT        
    // 000000000000  1    1         0        0          0       0       00     00    00101 01111
    // Reserved    ADCEN Unipolar NegBUFen PosBUFen PosBUFbp NegBUFbp INTREF CurOff  AIN5+ AGND
            pADI_ADC1->CON = 0x000c00af;
            pADI_ADC1->FLT = 0xc07d;
            pADI_ADC1->MDE = 0x0031;    //gain 8 +128 mV
#endif
}
float delta;
float mVtmp;
//-----------------------------------------------------------------------------
int adc_drv()
{
 //   if ((pADI_ADC0->STA & 0x01) && (pADI_ADC1->STA & 0x01))
    if ((pADI_ADC1->STA & 0x01))
    {
        ADC0_Val = pADI_ADC0->DAT;
        // 000000000000  1    0       0        0      0       0          00     00   01000 00111
        // Reserved    ADCEN Bipol NegBUFen PosBUFen PosBUFbp NegBUFbp INTREF CurOff AIN8+ AIN7-
   /*     pADI_ADC1->FLT = 0x9007;
        pADI_ADC1->CON = 0x00080107;
        pADI_ADC1->MDE = 0x0051;  //Gain = 32  ±22.18 mV*/
        //    pADI_ADC1->MDE = 0x0061;    //Gain = 64  ±10.3125 mV
        //    pADI_ADC1->MDE = 0x0071;    //Gain = 128  ±3.98 mV
        
        ADC1_Val = pADI_ADC1->DAT;
        delta = (ADC1_Val - ADC1_Val_Prev);
        ADC1_Val_Prev = ADC1_Val;            
        mVtmp = ADC1_Val * 1200. / 0x10000000;
        if(!((fabs(delta) < 100.) && (fabs(mVtmp) > 0.35)))
            return 0;  //ток на уровне шумов. Ну яво.        
        mV = ADC1_Val * 1200. / 0x10000000;       
        mV_2 = ADC0_Val * 1200. / 0x10000000;
        return 1;
/*               }
        else
        {
//           delta_consec_mes = ADC_Val - ADC_Val_Prev;
//            ADC_Val_Prev = ADC_Val;
//            delta_consec_mes = ADC_Val - ADC_Val_Prev;
//            ADC_Val_Prev = ADC_Val;
//            if((delta_consec_mes < 4) && (delta_consec_mes > -4))
//                return 0;  //ток на уровне шумов. Ну яво.
            

// 000000000000  1    0       0        0      0       0          00     00   00110 00101
// Reserved    ADCEN Bipol NegBUFen PosBUFen PosBUFbp NegBUFbp INTREF CurOff AIN6+ AIN5-
 //           pADI_ADC1->CON = 0x00080c05;
     //       pADI_ADC1->FLT = 0x9003;
            pADI_ADC1->FLT = 0x9007;
// 000000000000  1    0       1        1      1       1          00     00   00110 00101
// Reserved    ADCEN Bipol NegBUFen PosBUFen PosBUFbp NegBUFbp INTREF CurOff AIN6+ AIN5-
            pADI_ADC1->CON = 0x000bc0c5;
            pADI_ADC1->MDE = 0x0002;
      //      pADI_ADC1->CON = 0x000800c5;
      //      pADI_ADC1->MDE = 0x0022;

            conv = 1;
        }*/
    }
    return 0;
}

int adc_drv_tc()
{
    if ((pADI_ADC1->STA & 0x01) != 0)
    {
        ADC1_Val = pADI_ADC1->DAT;         
        mV = ADC1_Val;
        mV = mV * 1200. / 0x10000000;
        return 1;
    }
    return 0;
}