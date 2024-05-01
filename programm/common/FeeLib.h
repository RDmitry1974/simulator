/**
 *****************************************************************************
   @file     FeeLib.h
   @brief    Set of Flash peripheral functions.
   @version  V0.1
   @author   ADI
   @date     October 2012 

All files for ADuCM360/361 provided by ADI, including this file, are
provided  as is without warranty of any kind, either expressed or implied.
The user assumes any and all risk from the use of this code.
It is the responsibility of the person integrating this code into an application
to ensure that the resulting application performs as required and is safe.

**/
#ifndef _MCU_FREELIB_FILE_

#define _MCU_FREELIB_FILE_

#include <ADuCM361.h>

#define FA_KEYH (*(volatile unsigned long *) 0x1FFF4)
#define FA_KEYL (*(volatile unsigned long *) 0x1FFF0)
#define WR_PR (*(volatile unsigned long *) 0x1FFF8)


extern int FeeMErs(void);
extern int FeePErs(unsigned long lPage);
extern int FeeWrPro(unsigned long lKey);
extern int FeeWrProTmp(unsigned long lKey);
extern int FeeRdProTmp(int iMde);
extern int FeeWrEn(int iMde);
extern int FeeSta(void);
extern int FeeFAKey(unsigned long long udKey);
extern int FeeIntAbt(unsigned int iAEN0, unsigned int iAEN1, unsigned int iAEN2);
extern int FeeAbtAdr(void);
extern int FeeSign(unsigned long ulStartAddr, unsigned long ulEndAddr);
extern int FeeSig(void);

#endif