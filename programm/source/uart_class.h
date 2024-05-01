#pragma once

// mylib
//#include "processor.h"
//#include "state.h"
#include "aducm360.h"

#ifndef HARD_TEST
// Scheduler includes.
extern "C"
{
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"
}
#endif

enum UART_BAUD_RATE {BR_9600, BR_19200, BR_38400, BR_57600};

const UART_BAUD_RATE UART_SPEED = BR_38400;

#define  UART_TX_BUF_SIZE   250
#define  USART1_include
#define  UART1           pADI_UART
// *********************************************************************
enum Rx_Tx_dir { RX = 0, TX = 1 };

// *********************************************************************
/*enum UART_WordLen   { wl8bit = 0, wl9bit = 1 };
enum UART_StopBits  { sb1bit = 0, sb05bit = 1, sb2bit = 2, sb15bit = 3 };
enum UART_Parity    { prEven = 0, prOdd = 1, prNoParity = 2 };*/
/*struct My_USART_TypeDef
{
    __MY_REG32_BIT(SR,  __READ_WRITE ,__usart_sr_bits);
    __MY_REG32_BIT(DR,  __READ_WRITE ,__usart_dr_bits);
    __MY_REG32_BIT(BRR, __READ_WRITE ,__usart_brr_bits);
    __MY_REG32_BIT(CR1, __READ_WRITE ,__usart_cr1_bits);
    __MY_REG32_BIT(CR2, __READ_WRITE ,__usart_cr2_bits);
    __MY_REG32_BIT(CR3, __READ_WRITE ,__usart_cr3_bits);
    __MY_REG32_BIT(GTPR,__READ_WRITE ,__usart_gtpr_bits);
};
typedef volatile My_USART_TypeDef * const USART_TypeDefPtr;
*/
typedef volatile ADI_UART_TypeDef * const USART_TypeDefPtr;
struct Tuart_set
{
    unsigned int comdiv;
    unsigned int divm;
    unsigned int divn;
 //   float time_1_bit;
};

// *********************************************************************
//extern USART_TypeDefPtr UART1;
//extern USART_TypeDefPtr UART2;
//extern USART_TypeDefPtr UART3;

// *********************************************************************
class serial_device //: public shared_resource
{
public:
    virtual void init() = 0;
    virtual void config(UART_BAUD_RATE);
    virtual void set_dir(Rx_Tx_dir ) { } // full duplex - no transmitter

#ifndef HARD_TEST
    serial_device(): rx_queue_(NULL) {}

    virtual signed portBASE_TYPE send_buf(unsigned portCHAR *buf, uint32_t len) = 0;
    signed portBASE_TYPE get_char(uint8_t *val, portTickType TicksToWait)
    {
        set_dir(RX);
        return xQueueReceive(rx_queue_, val, TicksToWait);
    }

    signed portBASE_TYPE post_char(unsigned portCHAR cChar)
    {
        signed portBASE_TYPE WokenByPost = pdFALSE;
        xQueueSendFromISR(rx_queue_, &cChar, &WokenByPost);
        return WokenByPost;
    }

    virtual void init_queue()
    {
        rx_queue_ = xQueueCreate(UART_TX_BUF_SIZE, ( unsigned portBASE_TYPE ) sizeof( unsigned portCHAR ) );
    }
    xQueueHandle rx_queue() { return rx_queue_; }
    void empty_queue()
    {
        uint8_t buf;
        while (uxQueueMessagesWaiting(rx_queue_) != 0) 
            xQueueReceive(rx_queue_, &buf, 0);
    }

private:
    xQueueHandle rx_queue_;
#endif
};

// *********************************************************************
// *********************************************************************
class USART_device: public serial_device //: public shared_resource
{
public:
	USART_device(USART_TypeDefPtr USART_ptr): UART_ptr_(USART_ptr),
#ifndef HARD_TEST
    tx_queue_(NULL),
#endif
    direct_(RX) {}

    virtual void init_first() = 0;

#ifndef HARD_TEST
    void init_low(IRQn_Type IRQn, uint32_t priority);
#else
    void init_low();
#endif
    virtual void config(UART_BAUD_RATE);

#ifndef HARD_TEST
    void serial_interrupt();
 //   bool serial_interrupt();

    virtual signed portBASE_TYPE send_buf(unsigned portCHAR *buf, uint32_t len);

    virtual void init_queue()
    {
        serial_device::init_queue();
        tx_queue_ = xQueueCreate(UART_TX_BUF_SIZE, ( unsigned portBASE_TYPE ) sizeof(unsigned portCHAR ) );
    }
    xQueueHandle tx_queue() { return tx_queue_; }
#else
    bool recv() { return UART_ptr_->SR_bit.RXNE; }
    bool is_error()
    {
        return UART_ptr_->SR_bit.PE || UART_ptr_->SR_bit.FE
                    || UART_ptr_->SR_bit.NE || UART_ptr_->SR_bit.ORE;
    }
    bool recv_char() { return recv() && !is_error(); }

    uint8_t get_char() { return UART_ptr_->DR; }
    void send_char(uint8_t c) { UART_ptr_->DR = c; UART_ptr_->SR_bit.TC = 0; }
    bool send_ready() { return UART_ptr_->SR_bit.TXE; }
    bool sent() { return UART_ptr_->SR_bit.TC; }
#endif
private:
	USART_TypeDefPtr UART_ptr_;
#ifndef HARD_TEST
    xQueueHandle tx_queue_;
    xSemaphoreHandle transmit_;
#endif
    Rx_Tx_dir direct_;

    virtual void set_stop_bit() = 0;
    virtual void clr_stop_bit() = 0;
};

// *********************************************************************
#ifdef USART1_include
class USART_1: public USART_device
{
public:
	USART_1(): USART_device(UART1) {}
    void init();
    void init_first();

    virtual void set_dir(Rx_Tx_dir dir)
    {
#ifdef USART1_RX_TX_PIN
        GPIO_WriteBit(USART1_PORT, 1 << USART1_RX_TX_PIN, BOOL_PIN(dir == TX));
#else
        dir = dir;
#endif
    }

    virtual void set_stop_bit();
    virtual void clr_stop_bit();
};
extern USART_1 usart_1;
#endif



