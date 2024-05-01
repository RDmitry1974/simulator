#include "Mcu.h"

#define	CLK_CD0		0
#define	CLK_CD1		1
#define	CLK_CD2		2
#define	CLK_CD3		3
#define	CLK_CD4		4
#define	CLK_CD5		5
#define	CLK_CD6		6
#define	CLK_CD7		7

#define	CLK_HF		0
#define	CLK_LFX		1
#define	CLK_LF		2
#define	CLK_P4		3

#define	CLK_UCLKCG	0
#define	CLK_UCLK	1
#define	CLK_UDIV	2
#define	CLK_HFO		5
#define	CLK_LFO		6
#define	CLK_LFXO	7

//-----------------------------------------------------------------------------
void cpu_init()
{
    // Сконфигурировать Р0 + UART + дискретные входы
    pADI_GP0->GPCON = 0x003C;                              // configure as GPIO
    pADI_GP0->GPOEN = 0xfd;                                // configure as an input/output
    pADI_GP0->GPOCE = 0x00;                                // enable/disable open collector
    //pADI_GP0->GPPUL = 0x1F;                              // Подтяжка включена и для TXD
    // Сконфигурировать Р1
    pADI_GP1->GPCON = 0xa800;                              // SPI0 SCK + MOSI + CS
    pADI_GP1->GPOEN = 0xff;                                // configure as an output
    pADI_GP1->GPOCE = 0x00;                                // disable open collector
//    pADI_GP1->GPPUL = 0x3;                               // enable/disable the pull up
    // Сконфигурировать Р2
    pADI_GP2->GPCON = 0x0000;                              // SPI0 SCK + MOSI + CS
    pADI_GP2->GPOEN = 0xff;                                // configure as an output
    pADI_GP2->GPOCE = 0x02;                                // disable open collector

    //Disable clock to unused peripherals
	pADI_CLKCTL->CLKDIS = CLKDIS_DISSPI1CLK | CLKDIS_DISI2CCLK |
                                CLKDIS_DISPWMCLK | CLKDIS_DISDMACLK;

	pADI_CLKCTL->CLKCON0 = 0 | (CLK_HF << 3) | (CLK_UCLKCG << 5);
	pADI_CLKCTL->CLKSYSDIV = 0;

    pADI_CLKCTL->CLKCON1 = CLK_CD0 // SPI0
                            | (CLK_CD0 << 3) // SPI1
                            | (CLK_CD7 << 6) // I2C
                            | (CLK_CD0 << 9) // UART
                            | (CLK_CD7 << 12); // ?

//   10   0      0     1     0      0   1    0   1   0   0    1   0    1     1
//  MOD TFlush RFlush CON LoopBack OEN RXOF ZEN TIM LSB WOM CPOL CPHA MASEN ENABLE
    pADI_SPI0->SPICON = 0x894b;
    pADI_SPI0->SPIDIV = 3;  // divide by 2*(3+1) = 8
}

//-----------------------------------------------------------------------------
void dac_write(unsigned int val)
{
    pADI_SPI0->SPITX = (val >> 16) & 0xff;
    pADI_SPI0->SPITX = (val >> 8) & 0xff;
    pADI_SPI0->SPITX = val & 0xff;

    while ((pADI_SPI0->SPISTA & 0x20) == 0);
}
