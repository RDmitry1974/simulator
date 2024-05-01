#include "modbus.h"
#include "main.h"
//#include "outputs.h"
#include "adc.h"

float digital_out_value = 10.;

// ******************************************************************************************
// ******************************************************************************************
void CMODBUS::recv_byte(uint32_t symb)
{
	uint32_t st = get_state();
	if (st >= RX_TIMEOUT) return;
	else switch (st)
	{
    	case RTX_WAIT:
			recv_addr_ = symb;
			bufpos_ = 0;
			CRC16_Hi = CRC16_Lo = 0xFF;
			set_state(RX_RECV);
			count_CRC16(symb);
			break;

		case RX_RECV:
			get_buf()[bufpos_] = symb;
			set_state((++bufpos_ == 0xff) ? RX_MSG : RX_RECV);
			count_CRC16(symb);
			break;

		default:
			set_state(RTX_WAIT);
	}
}

// ******************************************************************************************
// ******************************************************************************************
void CMODBUSParser::read_coil(uint32_t , uint32_t )
{
    send_error(2);
}

// ******************************************************************************************
void CMODBUSParser::read_discret(uint32_t , uint32_t )
{
    send_error(2);
}

// ******************************************************************************************
// ******************************************************************************************
void CMODBUSParser::store_float(uint8_t *trg, float const val) const
{
    uint8_t *src = (uint8_t *)&val;
    trg[0] = src[1];
    trg[1] = src[0];
    trg[2] = src[3];
    trg[3] = src[2];
}

// ******************************************************************************************
float CMODBUSParser::retr_float(const uint8_t *target) const
{
	float result;
	uint8_t *src = (uint8_t *)target;
	uint8_t *trg = (uint8_t *)&result;

    trg[0] = src[1];
    trg[1] = src[0];
    trg[2] = src[3];
    trg[3] = src[2];

    return result;
}

// ******************************************************************************************
// ******************************************************************************************
void CMODBUSParser::read_regs(uint32_t addr, uint32_t amnt) 
{
	uint8_t *mess = get_buf();
    int index = 2;
    if(amnt > 3)
    {   //не может быть более 3 регистров
        send_error(2);
        return;
    }   
    if((addr == 0) && (amnt > 0))
    {   //0 регистр - тип канала
        mess[index++] = Tinput_output :: mb_type_ch >> 8;
        mess[index++] = Tinput_output :: mb_type_ch & 0xff;
        amnt--;
        addr++;
    }    
    if((addr == 1) && (amnt >= 2))
    {   //1 регистр - тип канала
        store_float(&mess[index], Tinput_output :: mb_value);
        index += 4;
        amnt -= 2;
        addr += 2;
    }  
    mess[1] = index - 2;
    send_message(index);
}

// ******************************************************************************************
//#pragma optimize=none
void CMODBUSParser::write_reg(uint32_t addr, uint32_t amnt)
{
//	uint8_t *mess = get_buf();
    if(addr == 0)
    {   //0 регистр - тип канала
        Tinput_output :: CHANNEL_TYPE ttt = (Tinput_output :: CHANNEL_TYPE)(amnt);
        if(ttt > Tinput_output::OFF_OUT)
        {
            send_error(2);
            return;
        }   
        taskENTER_CRITICAL();
        Tinput_output :: mb_type_ch = ttt;
        taskEXIT_CRITICAL();
 //       mess[1] = 4;
        send_message(6);
    }
    else
        send_error(2);
}

// ******************************************************************************************
//void CMODBUSParser::write_regs(uint32_t , uint32_t ) // (uint32_t addr, uint32_t amnt)
//#pragma optimize=none
void CMODBUSParser::write_regs(uint32_t addr, uint32_t amnt)
{
	uint8_t *mess = get_buf();
    if(addr == 0)
    {   //0 регистр - тип канала
        if ((get_buflen() < 12))
        {
            send_error(2);
            return;
        }   
        Tinput_output :: CHANNEL_TYPE ttt = (Tinput_output :: CHANNEL_TYPE)((mess[6] << 8) | mess[7]);
        if(ttt > Tinput_output::OFF_OUT)
        {
            send_error(2);
            return;
        }   
        taskENTER_CRITICAL();
        Tinput_output :: mb_type_ch = ttt;
        Tinput_output :: mb_value = retr_float(&(mess[8]));
        taskEXIT_CRITICAL();
  //      mess[0] = WRITE_REGS;                // func code
        send_message(6);
    }
    else if (addr == 1)
    {   //1 и 2 регистр - float значение канала
        taskENTER_CRITICAL();
        Tinput_output :: mb_value = retr_float(&(mess[6]));
        taskEXIT_CRITICAL();
        send_message(6);
    }
    else 
        send_error(2);
}

// ******************************************************************************************
// ******************************************************************************************
void CMODBUSParser::read_inps(uint32_t addr, uint32_t amnt)
{
    send_error(2);
/*	uint8_t *mess = get_buf();
    amnt <<= 1;                             // byte count
    if ((amnt > 0xf6) || (get_buflen() < 5))
        send_error(2);
    else
    {
        memset(mess, 0, amnt + 9);
        mess[1] = amnt;
        for (uint32_t i = 0; amnt > 0; ++addr, amnt -= 2, i += 2)
        {
            uint8_t hi = 0, lo = 0;
            if (addr < 0x000a)
            {
                switch (addr)
                {
                    case 0x00: lo = main_inp->get_state(); break;
                    case 0x02: lo = (DBIT1(HAS_OUTPUT) ? out_chan[0].get_err_state() : 2); break;
                    case 0x03: lo = (DBIT1(TWO_OUTPUT) && !D_MainOut[1].disabled()) ?
                                    out_chan[1].get_err_state() : 2; break;
                    case 0x04: store_float(mess + i + 2, main_inp->get_value()); break;
                    case 0x06: store_float(mess + i + 2, main_inp->get_raw()); break;
#ifdef TERMO
                    case 0x08: store_float(mess + i + 2, get_TCJ()); break;
#endif
                }
                if (addr < 4)
                {
                    mess[i + 2] = hi;
                    mess[i + 3] = lo;
                }
            }
        }
        mess[0] = READ_INPS;                // func code
        send_message(mess[1] + 2);
    }*/
}

// ******************************************************************************************
void CMODBUSParser::write_coil(uint32_t addr, uint32_t amnt)
{
    send_error(2);
 /*   bool val;
    if (amnt == 0xff00) val = true;
    else if (amnt == 0x0000) val = false;
    else
    {
        send_error(2);
        return;
    }

    if (addr == 0x000F)
    {
        if (val)
            D_MainCnf.nettype_ &= 0xFE;
        SET(COM_RECONF);
    }
    else
    {
        send_error(2);
        return;
    }

	uint8_t *mess = get_buf();
    mess[0] = WRITE_COIL;
    mess[1] = addr >> 8;
    mess[2] = addr & 0xff;
    mess[3] = amnt >> 8;
    mess[4] = amnt & 0xff;
    send_message(5);  // all codes are the same*/
}
uint8_t *mess;
// ******************************************************************************************
void CMODBUSParser::get_slave_ID()
{
//	uint8_t *mess = get_buf();
	mess = get_buf();
	mess[2] = 0;        // slave ID
	mess[3] = 0xff;     // run indicator
    char ID_VAL[] = "Calculon 4";
    strcpy((char*)(&mess[4]), ID_VAL);
    int len = strlen(ID_VAL);
	mess[1] = len + 2;        // 
    send_message(len + 4);
    
/*	uint8_t *mess = get_buf();
	mess[0] = SLAVE_ID;
	mess[1] = 3; 	// byte count
    uint32_t len = prepare_modify(mess + 3);
	mess[2] = mess[3];              // 'N' - slave ID
	mess[3] = 0xff; // run indicator
    send_message(3 + len);*/
}

// ******************************************************************************************
//#pragma optimize=none
bool CMODBUSParser::parse_mess()
{
    uint8_t *mess = get_buf();
    uint32_t addr = (mess[1] << 8) + mess[2];
    uint32_t amnt = (mess[3] << 8) + mess[4];
    switch (mess[0])
    {
        case READ_COILS:
            read_coil(addr, amnt);
            break;

        case READ_DISCRETE:
            read_discret(addr, amnt);
            break;

        case READ_REGS:
            read_regs(addr, amnt);
            break;

        case READ_INPS:
            read_inps(addr, amnt);
            break;

		case WRITE_REG:
			write_reg(addr, amnt);
            break;

        case WRITE_REGS:
        case WRITE_BYTES:
            write_regs(addr, amnt);
            break;

        case WRITE_COIL:
            write_coil(addr, amnt);
            break;

        case SLAVE_ID:
            get_slave_ID();
            break;

        default:
            send_error(1);
            break;
    }

    return is_mess();
}

// ******************************************************************************************
// ******************************************************************************************
void CMODBUSSlave::process_message()
{
	if (get_state() != RX_MSG)
	{
		set_state(RTX_WAIT);
		return;
	}

	uint32_t len = get_buf_pos();
    if (CRC16_Hi || CRC16_Lo || (len < 3))  // контр. сумма вместе с ЦРЦ
    {
    	set_state(RTX_WAIT);
		return;
	}

    parser()->set_buf(get_buf(), len - 2);
    if (parser()->parse_mess())
        send_message(parser()->get_buflen(), parser()->is_CRC());
    parser()->free();
}

