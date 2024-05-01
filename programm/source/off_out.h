#pragma once

#include "main.h"

//Отключить все выходные ножки

class TOffOut : public Tinput_output
{
public:
    void init();                    //инициализация АЦП, ЦАП, ввода вывода...
    void cycle_func();              //
private:
    void port_init();               //коммутация оптронов и тп
    void adc_init();                //настройка АЦП 
    void dac_init();                //настройка ЦАПов 
};

