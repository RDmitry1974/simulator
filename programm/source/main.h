
#pragma once

// Kernel includes.
extern "C"
{
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
}

//общий класс для работы с входами / выходами
class Tinput_output
{
public:
    enum CHANNEL_TYPE {CURR_ACTIV_SENSOR_OUT, CURR_PASSIV_SENSOR_OUT, TC_OUT, TR3_OUT, TR4_OUT, U_OUT, FREQ_OUT, CURR_INPUT, FREQ_OUT_SWITCH, OFF_OUT};
/*	class _Tnested // определяем вложенный класс с именем _nested
	{
	public:
		_Tnested() // конструктор _nested инициализирует нашу статическую переменную-член
		{
            vSemaphoreCreateBinary(xWaitCyclicSemaphore);
		}
	};     */    
    static CHANNEL_TYPE current_type_ch;    //тип текущего канала
    volatile static float value;            //значение выходного параметра
    static CHANNEL_TYPE mb_type_ch;         //тип текущего канала заданный по modbusu
    volatile static float mb_value;         //значение выходного параметра заданное по modbusu
    static int init_param;                  //переключатель между ТС3 и ТС4, Ток актив и ток пассив
    volatile static signed int ADC0_Val;
    volatile static signed int ADC1_Val;
    virtual void init();                    //инициализация АЦП, ЦАП, ввода вывода...
    virtual void cycle_func();              //
private:
//	static _Tnested semafore_initializer; // используем статический объект класса _Tnested для гарантии того, что конструктор _Tnested
protected:
    virtual bool is_ready_adc_conv();       //ждет окончания преобразования АЦП
    virtual void port_init();               //коммутация оптронов и тп
    virtual void adc_init();                //настройка АЦП 
    virtual void dac_init();                //настройка ЦАПов 
    static const float K_DIV_DAC;
    static const float R_CURR_SENS;
    static const float R_SENS;
    static float mV;
    static float mV_2;
    static unsigned int cycle_ctr;
    static float mVval, mVvalPrev;
    static unsigned int cf;
    static float mVE;
    static float delta_mV;
    static signed int ADC0_Val_Prev;
    static signed int ADC1_Val_Prev;
};
extern xSemaphoreHandle xWaitCyclicSemaphore;       //семафор в цикле ожидания какого-нить события, например окончание преобразования АЦП
