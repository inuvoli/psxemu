#include "cop0.h"
#include "cpu_short_pipe.h"

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
	reg[12] = 0x00200000;
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
		if (currentOperation.rt != 0)
			cpu->gpr[currentOperation.rt] = reg[currentOperation.rd];
			//if (currentOperation.rd == 12) printf("COP0 - Reading Status Register 0x%08x\n", cpu->gpr[currentOperation.rt]);
			//if (currentOperation.rd == 13) printf("COP0 - Reading Cause Register 0x%08x\n", cpu->gpr[currentOperation.rt]);
		break;

	case 0x02:	//cfc0 rt, rd
		printf("COP0 - Error! cfc0 is unsupported\n");
		break;

	case 0x04:	//mtc0 rt, rd
		reg[currentOperation.rd] = cpu->gpr[currentOperation.rt];
		//if (currentOperation.rd == 12) printf("COP0 - Writing Status Register 0x%08x\n", cpu->gpr[currentOperation.rt]);
		//if (currentOperation.rd == 13) printf("COP0 - Writing Cause Register 0x%08x\n", cpu->gpr[currentOperation.rt]);
		break;

	case 0x06:	//ctc0 rt, rd
		printf("COP0 - Error! ctc0 is unsupported\n");
		break;

	case 0x10:
		switch (currentOperation.funct)
		{
		case 0x01:	//tlbr
			printf("COP0 - Error! tlbr unsupported\n");
			break;
		case 0x02:	//tlbwi
			printf("COP0 - Error! tlbwi unsupported\n");
			break;
		case 0x06:	//tlbwr
			printf("COP0 - Error! tlbwr unsupported\n");
			break;
		case 0x08:	//tlbp
			printf("COP0 - Error! tlbp unsupported\n");
			break;
		case 0x10:	//rfe
			//Restore Interrupt Status (shift 2 position right bit [0, 5] for SR, bit 5 and 4 remain untouched)
			cop0::StatusRegister statusReg;
			statusReg.word = reg[12];
			statusReg.stk = ((statusReg.stk >> 2) & 0x0f) | (statusReg.stk & 0x30);
			reg[12] = statusReg.word;
			//printf("RFE -----------------------------------------------------------\n");
			break;
		}
		break;

	default:
		printf("COP0 - Unknown Sub Operation: 0x%02x\n", (unsigned int)currentOperation.operation);

	};

	return true;
}