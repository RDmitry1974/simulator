#pragma once

#include "main.h"

//Выход напряжения

class TUOut : public Tinput_output
{
public:
    void init();                    //инициализация АЦП, ЦАП, ввода вывода...
    void cycle_func();              //
private:
    bool is_ready_adc_conv();       //ждет окончания преобразования АЦП
    void port_init();               //коммутация оптронов и тп
    void adc_init();                //настройка АЦП 
    void dac_init();                //настройка ЦАПов 
    float delta_mV;
    float mVtmp;
};

