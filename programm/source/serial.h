#pragma once

#include <string.h>

#include "mymacro.h"
#include "uart_class.h"
//#include "ee_cnf.h"

// ********************************************************************
extern const uint8_t auchCRCHi[];
extern const uint8_t auchCRCLo[];

// ********************************************************************
enum Addr_Mode { SLAVE = 0, MASTER };
enum ProtState { RTX_WAIT = 0, READY, RX_HDR, RX_RECV, RX_MARK1, RX_MARK2,
                   RX_ENDCH, RX_END, RX_ADDR, RX_ADDR2, RX_CRC, RX_CRC2, RX_STOP,
                   RX_TIMEOUT, RX_CRC_ERR, RX_FE_ERR, RX_DOR_ERR, RX_UPE_ERR,
                   RX_MSG, RX_TX_BREAK,
                   TX_START, TX_DELAY, TX_FF, TX_FRAME, TX_HDR, TX_ADDR, TX_DATA,
                   TX_DCHK, TX_FTR, TX_END, TX_CRC, TX_CRC2, TX_CRC3, TX_LAST, TX_FINISH };

// ****************************************************
// ****************************************************
class shared_resource
{
public:
	shared_resource() { vSemaphoreCreateBinary( busy_ ); }  //if (xSemaphore != NULL)

    bool busy() const { return busy_; }
    bool take_delay(portTickType xBlockTime) { return xSemaphoreTake( busy_, xBlockTime); }
//    void take_wait() { xSemaphoreTake( busy_, 10); }
    void take_wait()
	{
		if (xTaskGetTickCount())
		{
			while (!xSemaphoreTake( busy_, 1)) __no_operation();
		}
	}
    bool take_async() { return xSemaphoreTake(busy_, 0); }
    void free() { if (xTaskGetTickCount()) xSemaphoreGive(busy_); }

private:
	xSemaphoreHandle busy_;
};

// *********************************************************************
// *********************************************************************
class CPureParser: public shared_resource
{
public:
    void set_buf(uint8_t * const buf, uint32_t const len)
    {
        take_wait();
        bufptr_ = buf;
        buflen_ = len;
        ready_ = add_CRC_ = false;
    }

    const uint8_t* get_buf() const { return bufptr_; }
    uint8_t* get_buf() { return bufptr_; }

    uint32_t  get_buflen() const { return buflen_; }
    bool is_mess()    const { return ready_; }
    bool is_CRC()     const { return add_CRC_; }

    virtual bool parse_mess() = 0;

protected:
    void send_message(uint32_t len, bool add_CRC)
    {
        buflen_ = len;
        ready_ = true;
        add_CRC_ = add_CRC;
    }
    void send_message(uint32_t len) { send_message(len, false); }

    virtual void send_error(uint32_t code) = 0;

private:
    uint8_t* bufptr_;
    uint32_t buflen_;
    bool ready_;
    bool add_CRC_;
};

// *********************************************************************
// *********************************************************************
//uint32_t prepare_modify(uint8_t * to_buf);

// *********************************************************************
class CUARTProtocol
{
public:
	static const uint32_t TimeoutSlave  = configTICK_RATE_HZ;
	static const uint32_t TimeoutMaster = configTICK_RATE_HZ / 5;

    virtual void process_message() = 0;

    uint32_t get_mess_len() { return buflen_; }

    void set_state(ProtState state) volatile
    {
        if (state < RX_TX_BREAK)
        {
            if ((addrmode_ == MASTER) && broadcast_ && (RTX_WAIT == state))
                state = RX_TIMEOUT;
        }
        state_ = state;
    }
    ProtState get_state() volatile  { return state_; }
#if defined(HARD_CONFIG) || defined(DSC_TEST)
    uint32_t  get_addr() const { return 31; }
#elif defined(RES_TEST)
    uint32_t  get_addr() const { return 30; }
#else
//    uint32_t  get_addr() const { return D_MainCnf.netnum_; }
    uint32_t  get_addr() const { return 1; }
#endif

    void set_device(serial_device *device) { device_ = device; }
    serial_device* get_device() const { return device_; }

    virtual uint8_t* get_buf() = 0;
    uint32_t get_buf_pos() { return bufpos_; }

    virtual void Init()
    {
        set_state(RTX_WAIT);
        device_->init();
    }

	void set_broad(bool broadcast) { broadcast_ = broadcast; }
    bool get_broad() const { return broadcast_; }

    void send_message(uint32_t len, bool add_CRC)
    {
        device_->empty_queue();
        if ((addrmode_ == MASTER) || !broadcast_)
        {
            uint8_t tx_buf[UART_TX_BUF_SIZE];

            if (add_CRC ? pack_message_CRC(tx_buf, len) : pack_message(tx_buf, len))
                device_->send_buf(tx_buf, buflen_);
		}

		broadcast_ = false;
    	set_state(RTX_WAIT);
	}
    void send_message(uint32_t len) { send_message(len, false); }

    virtual bool get_mess() = 0;
    virtual bool get_message(portTickType TicksToWait) = 0;
	bool send_imm(uint8_t *mess, uint32_t len, uint8_t attempts, portTickType TicksToWait)
	{
		for (uint32_t i = 0; i < attempts; ++i)
		{
            memcpy(get_buf(), mess, len);
			send_message(len);
            if (get_message(TicksToWait)) return true;
		}
		return false;
	}

	void set_addr_mode(Addr_Mode addrmode) { addrmode_ = addrmode; }
	Addr_Mode get_addr_mode() const { return addrmode_; }

protected:
	Addr_Mode addrmode_;
    CPureParser *parser_;
	bool broadcast_;
    serial_device *device_;
    uint32_t bufpos_, buflen_;
    volatile ProtState state_;
    uint8_t CRC16_Hi, CRC16_Lo;

 //   static const uint32_t      Speeds[7];
 //   static const UART_Parity   Parities[3];
 //   static const UART_StopBits StopBits[2];

    CPureParser *parser() const { return parser_; }

    void count_CRC16(uint8_t value)
	{
		uint32_t CRCInd = CRC16_Hi ^ value; // calculate the CRC
		CRC16_Hi = CRC16_Lo ^ auchCRCHi[CRCInd];
		CRC16_Lo = auchCRCLo[CRCInd];
	}

    virtual bool pack_message(uint8_t *buf, uint32_t len) = 0;
    virtual bool pack_message_CRC(uint8_t *buf, uint32_t len) { return pack_message(buf, len); }
};
