/**
 *****************************************************************************
   @file     IexcLib.h
   @brief    Set of Excitation Current Source functions.
   - Configure Excitation Currents with IexcCfg().
   - Select output current with IexcDat()

   @version  V0.1
   @author   ADI
   @date     March 2011 

All files for ADuCM360/361 provided by ADI, including this file, are
provided  as is without warranty of any kind, either expressed or implied.
The user assumes any and all risk from the use of this code.
It is the responsibility of the person integrating this code into an application
to ensure that the resulting application performs as required and is safe.

**/
#ifndef _MCU_IEXLIB_FILE_

#define _MCU_IEXLIB_FILE_

int IexcCfg(int iPd, int iRefsel, int iPinsel1, int iPinsel0);
int IexcDat(int iIDAT, int iIDAT0);

#define IDATVAL0uA  	0x0
#define IDATVAL50uA 	0x4
#define IDATVAL100uA  	0x5
#define IDATVAL150uA 	0x6
#define IDATVAL200uA  	0x7
#define IDATVAL250uA 	0x14
#define IDATVAL300uA  	0xA
#define IDATVAL400uA 	0xB
#define IDATVAL450uA  	0xE
#define IDATVAL500uA 	0x15
#define IDATVAL600uA 	0x1F
#define IDATVAL750uA  	0x16
#define IDATVAL800uA 	0x15
#define IDATVAL1mA 		0x1F
		
#define IDAT0En		1
#define IDAT0Dis	0

#endif


