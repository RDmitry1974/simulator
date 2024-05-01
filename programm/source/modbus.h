#pragma once

#include "serial.h"
//#include "ee_cnf.h"
#define MODBUS_BUF_LEN	        256
//#define MODBUS_BUF_LEN	        100

// ******************************************************************************************
// ******************************************************************************************
class CMODBUSParser: public CPureParser
{
public:
    virtual bool parse_mess();

protected:
    enum Commands
    {
        READ_COILS     = 0x01, READ_DISCRETE  = 0x02, READ_REGS      = 0x03,
        READ_INPS      = 0x04, WRITE_COIL     = 0x05, WRITE_REG      = 0x06,
        WRITE_COILS    = 0x0f, WRITE_REGS     = 0x10, SLAVE_ID		 = 0x11,
        READ_FILE      = 0x14, WRITE_FILE     = 0x15, MASK_WR_REG    = 0x16,
        R_W_REGS       = 0x17, READ_ID        = 0x2b, WRITE_BYTES 	 = 0x4d
    };

    virtual void send_error(uint32_t code)
    {
        get_buf()[0] |= 0x80 ;
        get_buf()[1] = code;
        send_message(2);
    }

    void store_float(uint8_t *target, float const val) const;
    float retr_float(const uint8_t *target) const;

    bool get_float(uint32_t addr, uint32_t *val);

    void read_coil(uint32_t addr, uint32_t amnt);
    void read_discret(uint32_t addr, uint32_t amnt);
    void read_regs(uint32_t addr, uint32_t amnt);
    void read_inps(uint32_t addr, uint32_t amnt);
    void write_reg(uint32_t addr, uint32_t amnt);
    void write_regs(uint32_t addr, uint32_t amnt);
    void write_coil(uint32_t addr, uint32_t amnt);
    void get_slave_ID();
};

// ******************************************************************************************
//extern CMODBUSParser mparser;

// ******************************************************
// ******************************************************************************************
class CMODBUS: public CUARTProtocol
{
public:
	void Init()
	{
		CUARTProtocol::Init();
		device_->config(UART_SPEED);
	}

    virtual bool get_message(portTickType TicksToWait)
    {
//        memset(get_buf(), 0, 100);
    	set_state(RTX_WAIT);
		do
		{
	        uint8_t val;
			if (device_->get_char(&val, get_state() == RX_RECV ? configTICK_RATE_HZ / 50 : TicksToWait) == pdPASS)
            {
                if (get_state() == RX_MSG)
                    return true;
                recv_byte(val);
            }
			else
			{
				if (get_state() == RX_RECV)
				{
					if (recv_addr_ == get_addr() || !recv_addr_ || (recv_addr_ == 0xff))
					{
						if (!CRC16_Hi && !CRC16_Lo && (bufpos_ > 2))
						{
							set_state(RX_MSG);
							return true;
						}
					}
				}
				set_state(RX_TIMEOUT);
			}
		}
        while (get_state() < RX_TIMEOUT);
        return false;
    }

    bool get_mess() { return get_message(configTICK_RATE_HZ * 5); }
    void recv_byte(uint32_t byte);

private:
	uint8_t recv_addr_;

    bool pack_message(uint8_t *tx_buf, uint32_t len)
    {
        uint8_t *buf = get_buf();
        CRC16_Hi = CRC16_Lo = 0xFF;
		count_CRC16(*tx_buf++ = get_addr());
        for (uint32_t i = 0; i < len; ++i) count_CRC16(*tx_buf++ = *buf++);
        *tx_buf++ = CRC16_Hi;
        *tx_buf++ = CRC16_Lo;
        buflen_ = len + 3;
        return buflen_ < 255;
    }
};

// ******************************************************************************************
class CMODBUSSlave: public CMODBUS
{
public:
    void init_dev(CPureParser *parser)
    {
        CMODBUS::Init();
        set_addr_mode(SLAVE);
        parser_ = parser;
    }
    uint8_t* get_buf() { return buf; }
    void process_message();

protected:
	uint8_t buf[MODBUS_BUF_LEN];
};

