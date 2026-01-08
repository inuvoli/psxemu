#include <loguru.hpp>
#include "interrupt.h"
#include "psx.h"

Interrupt::Interrupt()
{
    //Init Interrupt Registers
    i_stat = 0x0;
    i_mask = 0x0;
}

Interrupt::~Interrupt()
{

}

bool Interrupt::reset()
{
    //Reset Interrupt Registers
    i_stat = 0x0;
    i_mask = 0x0;

    return true;
}

bool Interrupt::execute()
{
    //Assert Hardware Interrupt 0 to CPU according to i_stat and i_mask value 
    psx->cpu->interrupt(static_cast<uint8_t>(cpu::hwInterrupt::int0), (i_stat & i_mask));

    return true;
}

bool Interrupt::request(uint32_t cause)
{
    //Set I_STAT Interrupt Flag according to the Interrupt Cause
	i_stat |= 1UL << cause;
        
    LOG_F(2, "INT - Request Hardware Interrupt (%s) [I_STAT: 0x%08x, I_MASK: 0x%08x, CAUSE: 0x%08x, STATUS: 0x%08x]", interruptDescription[cause].c_str(), i_stat, i_mask, psx->cpu->cop0->reg[13], psx->cpu->cop0->reg[12]);
    
    return true;
}
    
bool Interrupt::writeAddr(uint32_t addr, uint32_t& data, uint8_t bytes)
{
    cop0::CauseRegister		causeReg;

    switch (addr)
    {
    case 0x1f801070:
        i_stat &= data;
        LOG_F(3, "INT - Write I_STAT Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
        break;
    case 0x1f801074:
        i_mask = data &0x7ff;
        LOG_F(3, "INT - Write I_MASK Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
        break;
    default:
        LOG_F(ERROR, "INT - Write Unknown Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
        return false;
        break;
    }

    return true;
}

uint32_t Interrupt::readAddr(uint32_t addr, uint8_t bytes)
{	
    uint32_t data = 0;
		switch (addr)
		{
		case 0x1f801070:
			data = i_stat;
            LOG_F(3, "INT - Read I_STAT Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
			break;
		case 0x1f801074:
            data = i_mask;
            LOG_F(3, "INT - Read I_MASK Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
			break;
        default:
            LOG_F(ERROR, "INT - Read Unknown Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
            return 0x0;
            break;
		}

    return data;
}

