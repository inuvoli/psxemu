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

bool Interrupt::clock()
{
    //Set value for Hardware Interupt Pin to CPU
    //PSX only use Hardware Interrupt INT0
    uint8_t status = (i_stat & i_mask) ? 1 : 0;
    psx->cpu->interrupt(status);
       
    return true;
}

bool Interrupt::set(uint32_t cause)
{
    //Set I_STAT Interrupt Flag according to the Interrupt Cause
	i_stat |= 1UL << cause;

    //printf("%s requesting INTERRUPT\n", interruptDescription[cause].c_str());
    
    return true;
}
    
bool Interrupt::writeAddr(uint32_t addr, uint32_t& data, uint8_t bytes)
{
    switch (addr)
    {
    case 0x1f801070:
        i_stat &= data;
        //printf("INTERRUPT - Write Status:   0x%08x\n", i_stat);
        break;
    case 0x1f801074:
        i_mask = data &0x7ff;
        //printf("INTERRUPT - Write Mask:     0x%08x\n", i_mask);
        break;
    default:
        printf("INTERRUPT - Unknown Parameter Set addr: 0x%08x (%d)\n", addr, bytes);
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
            //printf("INTERRUPT - Read Status:   0x%08x\n", i_stat);
			break;
		case 0x1f801074:
            data = i_mask;
            //printf("INTERRUPT - Read Mask:     0x%08x\n", i_mask);
			break;
        default:
            printf("INTERRUPT - Unknown Parameter Get addr: 0x%08x (%d)\n", addr, bytes);
            break;
		}

    return data;
}

void Interrupt::getDebugInfo(InterruptDebugInfo& info)
{
    info.i_mask = i_mask;
    info.i_stat = i_stat;
}
