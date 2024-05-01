#pragma once

#include <stdint.h>

#define MHZ           *1000000l
#define KHZ           *1000l
#define HZ            *1l

#define _setL(port,bit) port&=~(1<<bit)
#define _setH(port,bit) port|=(1<<bit)
#define _set(port,bit,val) _set##val(port,bit)
#define on(x) _set (x)
#define SET(x) _setH(x)

#define _clrL(port,bit) port|=(1U<<bit)
#define _clrH(port,bit) port&=~(1U<<bit)
#define _clr(port,bit,val) _clr##val(port,bit)
#define off(x) _clr (x)
#define CLR(x) _clrH(x)

#define _bitL(port,bit) ((port&(1<<bit))==0)
#define _bitH(port,bit) ((port&(1<<bit))!=0)
#define _bit(port,bit,val) _bit##val(port,bit)
#define signal(x) _bit(x)
#define BIT1_   _bitH
#define BIT0_   _bitL
#define DBIT1(x) _bitH(x)
#define DBIT0(x) _bitL(x)

#define _cmpB(port,bit)   if (_bitL(port,bit)) _setH(port,bit); else _clrH(port,bit)
#define CMP(x) _cmpB(x)
#define myabs(x)  ( ((x) < 0) ? -(x) : (x) )

#define COPY_BIT(From,To,BitFrom,BitTo) { if (From&BitFrom) To|=BitTo; else To&=(~BitTo); }

#define STROBE_1(PIN)  { _clrH(PIN); _setH(PIN); _clrH(PIN); };
#define STROBE_0(PIN)  { _setH(PIN); _clrH(PIN); _setH(PIN); };
#define RISE_EDGE(PIN) { _clrH(PIN); _setH(PIN); };
#define FALL_EDGE(PIN) { _setH(PIN); _clrH(PIN); };

typedef enum { NO = 0, UP, DOWN, LEFT, RIGHT } DIRECTION;

// **************************************************************
inline uint32_t RollByte(uint32_t val, DIRECTION dir, uint32_t min, uint32_t max)
{
	if (val < min) return min; else if (val > max) return max;
	if ((UP == dir) || (LEFT == dir))
	{
		if (val > min) return val - 1; else return max;
	}
	else if ((DOWN == dir) || (RIGHT == dir))
	{
		if (val < max) return val + 1; else return min;
	}
	else return val;
}

// **************************************************************
inline uint32_t RollByteS(uint32_t val, DIRECTION dir, uint32_t min, uint32_t max)
{
	if (val < min) return min; else if (val > max) return max;
	if ((UP == dir) || (LEFT == dir))
	{
        if (val > min) return val - 1; else return min;
	}
	else if ((DOWN == dir) || (RIGHT == dir))
	{
        if (val < max) return val + 1; else return max;
	}
	else return val;
}


// *****************************************************************
// *****************************************************************
inline float max(float a, float b)
{
	return (a > b) ? a : b;
}

// *****************************************************************
inline float min(float a, float b)
{
	return (a > b) ? b : a;
}
