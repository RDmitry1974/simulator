#pragma once

#include <stdint.h>
#include "aducm360.h"
//#include "aducm361.h"

#define  __wdt_reload        pADI_WDT->T3CLRI = 0xCCCC    //  Reset the watchdog timer

#define KEY5   0x01
#define KEY1   0x08
#define KEY3   0x10
#define KEY6   0x20
#define KEY4   0x40
#define KEY2   0x80


#define OFF_I_REG 0x01
#define DAC_ATT 0x02

/*
P0.0    GPIO    выход   KEY5    0
P0.1    UART    вход    RXD
P0.2    UART    выход   TXD
P0.3    GPIO    выход   KEY1    0
P0.4    GPIO    выход   KEY3    0
P0.5    GPIO    выход   KEY6    0
P0.6    GPIO    выход   KEY4    0
P0.7    GPIO    выход   KEY2    0

//P1.1    GPIO    выход   KEY5    0

P1.5    SPI     выход   CLCK
P1.6    SPI     выход   MOSI
P1.7    SPI     выход   CS

P2.0    GPIO    выход   OFF_I_REG 0
P2.1    GPIO    выход   DAC_ATT 0
*/

//-----------------------------------------------------------------------------
void cpu_init();
void dac_write(unsigned int val);
