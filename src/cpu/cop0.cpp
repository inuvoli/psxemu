#include <loguru.hpp>
#include "cop0.h"
#include "cpu.h"

Cop0::Cop0(CPU* instance)
{
	//Link cop0 to cpu
	cpu = instance;

    //Reset Cop0 Registers
    std::memset(reg, 0x00, sizeof(uint32_t) * 32);

    //SR Initial Value: BEV is Set , TS is Set
	reg[12] = 0x00600000;
	//PRid Initial Value
	reg[15] = 0x00000002;
};

Cop0::~Cop0()
{

};

bool Cop0::reset()
{
    //Reset Cop0 Registers
    std::memset(reg, 0x00, sizeof(uint32_t) * 32);

    //SR Initial Value: BEV is Set , TS is Set
	reg[12] = 0x00600000;
	//PRid Initial Value
	reg[15] = 0x00000002;

	return true;
}

bool Cop0::execute(uint32_t cofun)
{
	cop0::Operation currentOperation;
	currentOperation.word = cofun;

	switch(currentOperation.operation)
	{
	case 0x00:	//mfc0 rt, rd
		LOG_F(3, "CP0 - mfc0 r%d, c0_r%d [0x%08x --> r%d]", (int)currentOperation.rt, (int)currentOperation.rd, reg[currentOperation.rd],  (int)currentOperation.rt);
		if (currentOperation.rt != 0)
			cpu->gpr[currentOperation.rt] = reg[currentOperation.rd];
		break;

	case 0x02:	//cfc0 rt, rd
		LOG_F(ERROR, "COP0 - Error! cfc0 is unsupported");
		break;

	case 0x04:	//mtc0 rt, rd
		LOG_F(3, "CP0 - mtc0 r%d, c0_r%d [0x%08x --> c0_r%d]", (int)currentOperation.rt, (int)currentOperation.rd, cpu->gpr[currentOperation.rt], (int)currentOperation.rd);
		switch(currentOperation.rd)
		{
			
			case 8: 	//BadVAddr ignore writes
			case 15:    //PRid id Read only ignore write
				break;
			
			case 13:	//CauseRegister: Only BD, ExcCode and IP7-IP2 bits are writable
				reg[currentOperation.rd] = cpu->gpr[currentOperation.rt] & 0x8000fc7c;
				break;

			default:
				reg[currentOperation.rd] = cpu->gpr[currentOperation.rt];
				break;
		}
		break;

	case 0x06:	//ctc0 rt, rd
		LOG_F(ERROR, "COP0 - Error! ctc0 is unsupported");
		break;

	case 0x10:
		switch (currentOperation.funct)
		{
		case 0x01:	//tlbr
			LOG_F(ERROR, "COP0 - Error! tlbr unsupported");
			break;
		case 0x02:	//tlbwi
			LOG_F(ERROR, "COP0 - Error! tlbwi unsupported");
			break;
		case 0x06:	//tlbwr
			LOG_F(ERROR, "COP0 - Error! tlbwr unsupported");
			break;
		case 0x08:	//tlbp
			LOG_F(ERROR, "COP0 - Error! tlbp unsupported");
			break;
		case 0x10:	//rfe
			//Restore Interrupt Status (shift 2 position right bit [0, 5] for SR, bit 5 and 4 remain untouched)
			cop0::StatusRegister statusReg;
			statusReg.word = reg[12];
			statusReg.stk = ((statusReg.stk >> 2) & 0x0f) | (statusReg.stk & 0x30);
			reg[12] = statusReg.word;
			LOG_F(2, "COP0 - Executed RFE [PC: 0x%08x, EPC: 0x%08x, CauseRegister: 0x%08x, StatusRegister: 0x%08x]", cpu->pc, reg[14], reg[13], reg[12]);

			break;
		}
		break;

	default:
		LOG_F(ERROR, "COP0 - Unknown Sub Operation: 0x%02x", (unsigned int)currentOperation.operation);

	};

	return true;
}