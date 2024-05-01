
#include "main.h"
#include "mcu.h"
#include "adc.h"
#include "tr_out.h"
#include "tc_out.h"
#include "cur_out.h"
#include "cur_in.h"
#include "freq_out.h"
#include "off_out.h"
#include "u_out.h"
#include <math.h>
#include "modbus.h"
#include <intrinsics.h>



float Rset = 100.;

//-----------------------------------------------------------------------------
const float Tinput_output :: K_DIV_DAC = 101./(101. + 5110.);
const float Tinput_output :: R_CURR_SENS = 51.1;
const float Tinput_output :: R_SENS = 10.1;

//-----------------------------------------------------------------------------
Tinput_output :: CHANNEL_TYPE Tinput_output :: current_type_ch = Tinput_output :: TR4_OUT;
Tinput_output :: CHANNEL_TYPE Tinput_output :: mb_type_ch = Tinput_output :: TR4_OUT;
volatile float Tinput_output :: value = 100.;
volatile float Tinput_output :: mb_value = 100.;
float Tinput_output :: mV = 0;
float Tinput_output :: mV_2 = 0;
unsigned int Tinput_output :: cycle_ctr = 0;
float Tinput_output :: mVval = 0;
float Tinput_output :: mVvalPrev = 0;
unsigned int Tinput_output :: cf = 0;
float Tinput_output :: mVE = 0;
float Tinput_output :: delta_mV = 0;
volatile signed int Tinput_output :: ADC0_Val = 0;
signed int Tinput_output :: ADC0_Val_Prev = 0;
volatile signed int Tinput_output :: ADC1_Val = 0;
signed int Tinput_output :: ADC1_Val_Prev = 0;
int Tinput_output :: init_param = 0;
xSemaphoreHandle xWaitCyclicSemaphore;
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CMODBUSSlave mod_bus;
CMODBUSParser mod_bus_parser;
//-----------------------------------------------------------------------------
Tinput_output* obj_ptr;
TTROut tr_out;
TTCOut tc_out;
TCurOut cur_out;
TCurIn cur_in;
TFreqOut freq_out;
TOffOut off_out;
TUOut u_out;


//-----------------------------------------------------------------------------
//#pragma optimize=none
extern "C" void ADC1_Int_Handler( void );
void ADC1_Int_Handler( void )
{
    Tinput_output :: ADC0_Val = pADI_ADC0->DAT;
    Tinput_output :: ADC1_Val = pADI_ADC1->DAT;
    xSemaphoreGiveFromISR(xWaitCyclicSemaphore, NULL);
}
bool is_port_pin = false;
//-----------------------------------------------------------------------------
//#pragma optimize=none
extern "C" void GP_Tmr0_Int_Handler( void );
void GP_Tmr0_Int_Handler( void )
{
    pADI_TM0->CLRI = 0x1;   // сбросить флаг
    __DSB();//asmDSB(); 
        is_port_pin = !is_port_pin;
        if(obj_ptr->init_param)
        {
            if(is_port_pin)
                pADI_GP0->GPSET = KEY6;
            else
                pADI_GP0->GPCLR = KEY6;
        }
        else
        {
            if(is_port_pin)
                dac_write(0x003ffff0);
            else
                dac_write(0x00300000);
        }
    xSemaphoreGiveFromISR(xWaitCyclicSemaphore, NULL);
}

//-----------------------------------------------------------------------------
void vModBusTask( void *pvParameters )
{
    mod_bus.set_device(&usart_1);
    mod_bus.init_dev(&mod_bus_parser);
    mod_bus.Init();
    usart_1.init_first();
    
    while (1)
    {
        if(mod_bus.get_mess())
            mod_bus.process_message();
    }
}
//-----------------------------------------------------------------------------
//#pragma optimize=none
void vMainTask1( void *pvParameters )
{
    while (1)
        __no_operation();
}
//-----------------------------------------------------------------------------
void vMainTask( void *pvParameters )
{
    Tinput_output :: current_type_ch = Tinput_output :: TR4_OUT;
    Tinput_output :: mb_type_ch = Tinput_output :: current_type_ch;
  //  ADC1_Int_Handler();
  //  Tinput_output :: current_type_ch = Tinput_output :: TR3_OUT;
  //  Tinput_output :: current_type_ch = Tinput_output :: TC_OUT;
  //  Tinput_output :: current_type_ch = Tinput_output :: CURR_PASSIV_SENSOR_OUT;
  //  Tinput_output :: current_type_ch = Tinput_output :: FREQ_OUT;
 //   Tinput_output :: current_type_ch = Tinput_output :: CURR_INPUT;
    while (1)
    {
        pADI_TM0->CON = 0x00;           // остановить таймер
        switch(Tinput_output :: current_type_ch)
        {
        case Tinput_output :: TR4_OUT:
            obj_ptr = &tr_out;
            obj_ptr->init_param = 0;
            break;
        case Tinput_output :: TR3_OUT:
            obj_ptr = &tr_out;
            obj_ptr->init_param = 1;
            break;
        case Tinput_output::CURR_ACTIV_SENSOR_OUT:
            obj_ptr = &cur_out;
            obj_ptr->init_param = 1;
            break;
        case Tinput_output::CURR_PASSIV_SENSOR_OUT:
            obj_ptr = &cur_out;
            obj_ptr->init_param = 0;    
            break;
        case Tinput_output::TC_OUT:
            obj_ptr = &tc_out;
            break;
        case Tinput_output::U_OUT:
            obj_ptr = &u_out;
            break;
        case Tinput_output::FREQ_OUT:
            obj_ptr = &freq_out;
            obj_ptr->init_param = 0;    //амплитуда выходного сигнала 0 - 2400мВ
            break;
        case Tinput_output::CURR_INPUT:
            obj_ptr = &cur_in;
            break;            
        case Tinput_output::OFF_OUT:
            obj_ptr = &off_out;
            break;            
        case Tinput_output::FREQ_OUT_SWITCH:
            obj_ptr = &freq_out;
            obj_ptr->init_param = 1;    //выходной сигнал сухой контакт
            break;
        }
        obj_ptr->init();
        while (1)
        {
            obj_ptr->cycle_func();
            __wdt_reload;
            taskENTER_CRITICAL();
            if(Tinput_output :: current_type_ch != Tinput_output::CURR_INPUT)
                obj_ptr->value = Tinput_output :: mb_value;
            else
                Tinput_output :: mb_value = obj_ptr->value;
            volatile Tinput_output :: CHANNEL_TYPE ttt = Tinput_output :: mb_type_ch;
            taskEXIT_CRITICAL();
            if(Tinput_output :: current_type_ch != ttt)
            {
                Tinput_output :: current_type_ch = ttt;
                break;
            }
        }
    }
}
//-----------------------------------------------------------------------------
portBASE_TYPE dummers;

void main()
{
    //WdtCfg(T3CON_PRE_DIV16,T3CON_IRQ_DIS,T3CON_PD_DIS); //  Watchdog timer setup
    
    cpu_init();
    vSemaphoreCreateBinary(xWaitCyclicSemaphore);
    //WdtGo(T3CON_ENABLE_EN);                              //  Enable the watchdog timer
    dummers = xTaskCreate(vMainTask, "MTask", 100, NULL, 0, NULL);
    dummers = xTaskCreate(vModBusTask, "MBTask", 500, NULL, 0, NULL); //
    vTaskStartScheduler();
}


