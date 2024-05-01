#include "uart_class.h"
//#include "processor.h"

// *********************************************************************
// *********************************************************************
// Table 166. Baud Rate Examples—UCLK/DIV = 16 MHz for All Examples
// *********************************************************************
// Baud Rates  COMDIV  DIVM  DIVN  Actual     % Error
// *********************************************************************
// 9600        17      3     131  9599.25     -0.0078%
// 19,200      8       3     523  19,199.04   -0.0050%
// 38,400      4       3     523  38,398.08   -0.0050%
// 57,600      8       1     174  57,605.76   +0.0100%
// 115,200     2       2     348  115,211.5   +0.0100%
// 230,400     2       1     174  230,423     +0.0100%
// 460,800     1       1     174  460,846.1   +0.0100%
const Tuart_set UART_SET[] = 
{
    17, 3,  131,
    8,  3,  523,
    4,  3,  523,
    8,  1,  174
};


// ********************************************************************
void USART_device::config(UART_BAUD_RATE br)
{
 //   UART_ptr_ = ADI_UART_ADDR;
//    TIMER_ptr_ = pADI_TM1;
    UART_ptr_->COMFBR = COMFBR_ENABLE | (UART_SET[br].divm << 11) | UART_SET[br].divn; 
    UART_ptr_->COMDIV = UART_SET[br].comdiv;                                  //
    
    UART_ptr_->COMIIR = 3;                                  //Transmit buffer empty interrupt, Receive buffer full interrupt
    UART_ptr_->COMLCR = COMLCR_WLS_8BITS;
    UART_ptr_->COMCON = COMCON_DISABLE_EN;                  //включить UART
    //  000        0      0000  0  00    1      1  0  10
    //Reserved  EVENTEN  EVENT RLD CLK ENABLE  MOD UP PRE
//    while(TIMER_ptr_->STA & 0xC0)__no_operation();
//    TIMER_ptr_->CON = 0x00;           // остановить таймер
//    TIMER_ptr_->LD = (unsigned int)((16.0e3 /16.) * 20 * UART_SET[br].time_1_bit);  //выдержка 2 байта
//    TIMER_ptr_->CON = 0x89;           // настроить таймер /16
}

// не совсем приоритетное прерывание :-)
#define UART_IRQ_PRIORITY  (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1)

// *********************************************************************
#ifndef HARD_TEST
void USART_device::init_low(IRQn_Type IRQn, uint32_t priority)
#else
void USART_device::init_low()
#endif
{
//    shared_resource::init();
    // Create the queues used to hold Rx and Tx uint8_tacters.
#ifndef HARD_TEST
    //rx_queue_ = xQueueCreate(UART_TX_BUF_SIZE, ( unsigned portBASE_TYPE ) sizeof( unsigned portCHAR ) );
    init_queue();

    vSemaphoreCreateBinary(transmit_);
    xSemaphoreTake( transmit_, 0);
#endif
    direct_ = RX;

    set_dir(RX);
#ifndef HARD_TEST
    NVIC_EnableIRQ(IRQn);
    NVIC_SetPriority(IRQn, priority);

    // enable RX Data Available interrupt
    UART_ptr_->COMIEN |= 1;  //Receive buffer full interrupt
#else
    // disable RX Data Available interrupt
    UART_ptr_->COMIEN &= ~1;
#endif
    UART_ptr_->COMCON = 0; //Enable the UART
}

// ********************************************************************
#ifndef HARD_TEST
void USART_device::serial_interrupt()
{
    volatile portCHAR source = UART_ptr_->COMIIR;
//    unsigned char cChar;

    if ((source & 0x06) == 0x04)
    {   // Receive buffer full interrupt
        unsigned portCHAR cChar = UART_ptr_->COMRX;
        post_char(cChar);
    }

    if ((source & 0x06) == 0x02)
    {   // Transmit buffer empty interrupt
 //       UART_ptr_->SR_bit.TC = 0;
        if (direct_ == TX)
        {
            unsigned portCHAR cChar;
            if (xQueueReceiveFromISR(tx_queue_, &cChar, NULL) == pdTRUE)
            {
                //          UART_ptr_->SR_bit.TC = 0;
                UART_ptr_->COMTX = cChar;
            }
            else
            {
                set_stop_bit();
    //            UART_ptr_->COMTX = 0xff;
                direct_ = RX;
            }
        }
        else
        {
            UART_ptr_->COMIEN &= ~2;    //disable Transmit buffer empty interrupt
            set_dir(RX);
            clr_stop_bit();
            xSemaphoreGiveFromISR(transmit_, NULL);
        }
    }
}
/*bool USART_device::serial_interrupt()
{
    signed portBASE_TYPE WokenByTx = pdFALSE, WokenByPost = pdFALSE, WokenByEnd = pdFALSE;
    volatile portCHAR source = UART_ptr_->COMIIR;
//    unsigned char cChar;

    if ((source & 0x06) == 0x04)
    {   // Receive buffer full interrupt
        unsigned portCHAR cChar = UART_ptr_->COMRX;
        WokenByPost = post_char(cChar);
    }

    if ((source & 0x06) == 0x02)
    {   // Transmit buffer empty interrupt
 //       UART_ptr_->SR_bit.TC = 0;
        if (direct_ == TX)
        {
            unsigned portCHAR cChar;
            if (xQueueReceiveFromISR(tx_queue_, &cChar, &WokenByTx) == pdTRUE)
            {
                //          UART_ptr_->SR_bit.TC = 0;
                UART_ptr_->COMTX = cChar;
            }
            else
            {
                set_stop_bit();
                UART_ptr_->COMTX = 0xff;
                direct_ = RX;
            }
        }
        else
        {
            UART_ptr_->COMIEN &= ~2;    //disable Transmit buffer empty interrupt
            set_dir(RX);
            clr_stop_bit();
            xSemaphoreGiveFromISR(transmit_, &WokenByEnd);
        }
    }
    return WokenByTx || WokenByPost || WokenByEnd;
}*/
#define   serNO_BLOCK  0
// ********************************************************************
signed portBASE_TYPE USART_device::send_buf(unsigned portCHAR *buf, uint32_t len)
{
	if (len > UART_TX_BUF_SIZE) return pdFAIL;

	for (uint32_t i = 1; i < len; ++i)
		if (xQueueSend(tx_queue_, buf + i, serNO_BLOCK) != pdPASS) return pdFAIL;

	set_dir(TX);
    UART_ptr_->COMTX = *buf;
    direct_ = TX;

    UART_ptr_->COMIEN |= 2;    //enable Transmit buffer empty interrupt
  //  UART_ptr_->CR1_bit.TXEIE = 1;
  //  UART_ptr_->CR1_bit.TCIE = 0;

    xSemaphoreTake( transmit_, configTICK_RATE_HZ / 4);
	return pdPASS;
}
#endif

#ifdef USART1_include
// ********************************************************************
// ********************************************************************
void USART_1::init()
{
  /*  RCC_APB2ENR_bit.USART1EN = 1;

    // Setup the UART port pins
    GPIO_InitTypeDef  init_str;

    init_str.GPIO_Mode  = GPIO_Mode_AF;
    init_str.GPIO_Speed = GPIO_Speed_40MHz;
    init_str.GPIO_OType = GPIO_OType_PP;
    init_str.GPIO_PuPd  = GPIO_PuPd_UP;

    init_str.GPIO_Pin = (1 << USART1_RX_PIN) | (1 << USART1_TX_PIN);
    GPIO_Init(USART1_PORT, &init_str);

    GPIO_PinAFConfig(USART1_PORT, USART1_TX_PIN, GPIO_AF_USART1);
    GPIO_PinAFConfig(USART1_PORT, USART1_RX_PIN, GPIO_AF_USART1);*/

#ifdef USART1_RX_TX_PIN
    init_str.GPIO_Pin  = 1 << USART1_RX_TX_PIN;
    init_str.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_Init(USART1_PORT, &init_str);
#endif
}

void USART_1::init_first()
{
    init();

#ifndef HARD_TEST
    USART_device::init_low(UART_IRQn, UART_IRQ_PRIORITY);
#else
    USART_device::init_low();
#endif
}

// ********************************************************************
void USART_1::set_stop_bit()
{
 //   USART1_PORT->MODER &= ~(3UL << (USART1_TX_PIN * 2));
 //   USART1_PORT->MODER |= GPIO_Mode_OUT << (USART1_TX_PIN * 2);
 //   USART1_PORT->BSRRL = 1 << USART1_TX_PIN;
}

// ********************************************************************
void USART_1::clr_stop_bit()
{
 //   USART1_PORT->MODER &= ~(3UL << (USART1_TX_PIN * 2));
 //   USART1_PORT->MODER |= GPIO_Mode_AF << (USART1_TX_PIN * 2);
}

// ********************************************************************
// The interrupt service routine - called from the assembly entry point.
USART_1 usart_1;
//extern USART_1 usart_1;
#ifndef HARD_TEST
extern "C" void UART_Int_Handler( void );
void UART_Int_Handler( void )
{
	usart_1.serial_interrupt();
    // If a task was woken by either a uint8_tacter being received or a uint8_tacter
	// being transmitted then we may need to switch to another task.
	//portEND_SWITCHING_ISR(usart_1.serial_interrupt());
}
#endif

#endif // USART1

