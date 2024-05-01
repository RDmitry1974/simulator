#pragma once

#include "main.h"

//Токовый выход

class TFreqOut : public Tinput_output
{
public:
    void init();                    //инициализация АЦП, ЦАП, ввода вывода...
    void cycle_func();              //

private:
    void reload_timer(float val);
    void port_init();               //коммутация оптронов и тп
    void adc_init();                //настройка АЦП 
    void dac_init();                //настройка ЦАПов 
    float prev_value;               //предыдущее значение
  //  bool is_port_pin;               //состояние ножки проца
};

