#include "cpu_short_pipe.h"
#include "psx.h"
#include "common.h"

CPU::CPU()
{
	//Init CPU Registers
	pc = 0xbfc00000;
	hi = 0x00000000;
	lo = 0x00000000;
	memset(gpr, 0x00, sizeof(uint32_t) * 32);
	memset(cop0_reg, 0x00, sizeof(uint32_t) * 32);
	memset(cop1_reg, 0x00, sizeof(uint32_t) * 32);
	memset(cop2_reg, 0x00, sizeof(uint32_t) * 32);
	memset(cop3_reg, 0x00, sizeof(uint32_t) * 32);
	cacheReg = 0x00000000;
	interruptStatus = 0x00000000;
	interruptMask = 0x00000000;

	//Cop0 SR Initial Value: BEV is Set , TS is Set
	cop0_reg[12] = 0x00600000;
	//Cop0 PRid Initial Value
	cop0_reg[15] = 0x00000002;

	//Init Pipeline Registers and Status
	memset(&currentOpcode, 0x00, sizeof(decodedOpcode));
	stallPipeline = false;
	dmaTakeOnBus = false;
	branchDelaySlot = false;
	branchAddress = 0x00000000;

	//Init Instruction & Function Dictionaries
	instrSet =
	{
		{"special", &CPU::op_unknown},
		{"bxx rs, imm", &CPU::op_bxx},
		{"j tgt", &CPU::op_j},
		{"jal tgt", &CPU::op_jal},
		{"beq rs, rt, imm", &CPU::op_beq},
		{"bne rs, rt, imm", &CPU::op_bne},
		{"blez rs, imm", &CPU::op_blez},
		{"bgtz rs, imm", &CPU::op_bgtz},
		{"addi rt, rs, imm", &CPU::op_addi},
		{"addiu rt, rs, imm", &CPU::op_addiu},
		{"slti rt, rs, imm", &CPU::op_slti},
		{"sltiu rt, rs, imm", &CPU::op_sltiu},
		{"andi rt, rs, imm", &CPU::op_andi},
		{"ori rt, rs, imm", &CPU::op_ori},
		{"xori rt, rs, imm", &CPU::op_xori},
		{"lui rt, imm", &CPU::op_lui},
		{"cop0 cofun", &CPU::op_cop0},
		{"cop1 cofun", &CPU::op_cop1},
		{"cop2 cofun", &CPU::op_cop2},
		{"cop3 cofun", &CPU::op_cop3},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"lb rt, imm(rs)", &CPU::op_lb},
		{"lh rt, imm(rs)", &CPU::op_lh},
		{"lwl rt, imm(rs)", &CPU::op_lwl},
		{"lw rt, imm(rs)", &CPU::op_lw},
		{"lbu rt, imm(rs)", &CPU::op_lbu},
		{"lhu rt, imm(rs)", &CPU::op_lhu},
		{"lwr rt, imm(rs)", &CPU::op_lwr},
		{"invalid operation", &CPU::op_unknown},
		{"sb rt, imm(rs)", &CPU::op_sb},
		{"sh rt, imm(rs)", &CPU::op_sh},
		{"swl rt, imm(rs)", &CPU::op_swl},
		{"sw rt, imm(rs)", &CPU::op_sw},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"swr rt, imm(rs)", &CPU::op_swr},
		{"invalid operation", &CPU::op_unknown},
		{"lwc0 rt, imm(rs)", &CPU::op_lwc0},
		{"lwc1 rt, imm(rs)", &CPU::op_lwc1},
		{"lwc2 rt, imm(rs)", &CPU::op_lwc2},
		{"lwc3 rt, imm(rs)", &CPU::op_lwc3},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"swc0 rt, imm(rs)", &CPU::op_swc0},
		{"swc1 rt, imm(rs)", &CPU::op_swc1},
		{"swc2 rt, imm(rs)", &CPU::op_swc2},
		{"swc3 rt, imm(rs)", &CPU::op_swc3},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown}
	};

	functSet =
	{
		{"sll rd, rt, shamt", &CPU::op_sll},
		{"invalid operation", &CPU::op_unknown},
		{"srl rd, rt, shamt", &CPU::op_srl},
		{"sra rd,rt, shamt", &CPU::op_sra},
		{"sllv rd, rt, rs", &CPU::op_sllv},
		{"invalid operation", &CPU::op_unknown},
		{"srlv rd, rt, rs", &CPU::op_srlv},
		{"srav rd, rt, rs", &CPU::op_srav},
		{"jr rs", &CPU::op_jr},
		{"jalr rd, rs", &CPU::op_jalr},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"syscall", &CPU::op_syscall},
		{"break", &CPU::op_break},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"mfhi rd", &CPU::op_mfhi},
		{"mthi rs", &CPU::op_mthi},
		{"mflo rd", &CPU::op_mflo},
		{"mtlo rs", &CPU::op_mtlo},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"mult rs, rt", &CPU::op_mult},
		{"multu rs, rt", &CPU::op_multu},
		{"div rs, rt", &CPU::op_div},
		{"divu rs, rt", &CPU::op_divu},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"add rd, rs, rt", &CPU::op_add},
		{"addu rd, rs, rt", &CPU::op_addu},
		{"sub rd, rs, rt", &CPU::op_sub},
		{"subu rd, rs, rt", &CPU::op_subu},
		{"and rd, rs, rt", &CPU::op_and},
		{"or rd, rs, rt", &CPU::op_or},
		{"xor rd, rs, rt", &CPU::op_xor},
		{"nor rd, rs, rt", &CPU::op_nor},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"slt rd, rs, rt", &CPU::op_slt},
		{"sltu rd, rs, rt", &CPU::op_sltu},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown},
		{"invalid operation", &CPU::op_unknown}
	};
}

CPU::~CPU()
{
}

bool CPU::reset()
{
	//Init CPU Registers
	pc = 0xbfc00000;
	hi = 0x00000000;
	lo = 0x00000000;
	memset(gpr, 0x00, sizeof(uint32_t) * 32);
	memset(cop0_reg, 0x00, sizeof(uint32_t) * 32);
	memset(cop1_reg, 0x00, sizeof(uint32_t) * 32);
	memset(cop2_reg, 0x00, sizeof(uint32_t) * 32);
	memset(cop3_reg, 0x00, sizeof(uint32_t) * 32);
	cacheReg = 0x00000000;
	interruptStatus = 0x00000000;
	interruptMask = 0x00000000;

	//Cop0 SR Initial Value: BEV is Set , TS is Set
	cop0_reg[12] = 0x00600000;
	//Cop0 PRid Initial Value
	cop0_reg[15] = 0x00000002;

	//Init Pipeline Registers and Status
	memset(&currentOpcode, 0x00, sizeof(decodedOpcode));
	stallPipeline = false;
	dmaTakeOnBus = false;
	branchDelaySlot = false;
	branchAddress = 0x00000000;

	return true;
}

//-----------------------------------------------------------------------------------------------------------------------------------
//
// Helper Functions
//
//-----------------------------------------------------------------------------------------------------------------------------------
inline bool CPU::isOverflow(int32_t a, int32_t b)
{
	int32_t result;
	result = a + b;
	if (a > 0 && b > 0 && result < 0)
		return true;
	if (a < 0 && b < 0 && result > 0)
		return true;
	return false;
}

//-----------------------------------------------------------------------------------------------------------------------------------
//
// Cache and Memory Access Functions
//
//-----------------------------------------------------------------------------------------------------------------------------------
inline uint32_t CPU::rdInst(uint32_t vAddr, uint8_t bytes)
{
	//Check if PC in unaligned
	if ((bool)(vAddr % bytes))
		exception(static_cast<uint32_t>(exceptionCause::addrerrload));

	//TODO: Instruction Cache Management

	return psx->rdMem(vAddr, bytes);
}

inline uint32_t CPU::rdMem(uint32_t vAddr, uint8_t bytes, bool checkalign)
{
	StatusRegister statusReg;

	statusReg.word = cop0_reg[12];

	//Check if vAddr in unaligned
	if ((bool)(vAddr % bytes) && checkalign)
		exception(static_cast<uint32_t>(exceptionCause::addrerrload));

	//Check if Cache is Isolated
	if (statusReg.isc)
	{
		//TODO
	}

	//Check if Reading from Data Cache, aka ScratchPad (0x1f800000 - 0x1f8003ff)
	if (isInRange(vAddr, 0x1f800000, 0x1f800400))
	{
		return rdDataCache(vAddr, bytes);
	}

	//Check if Reading from Interrupt Control Registers (0x1f801070 - 0x1f801074)
	if (isInRange(vAddr, 0x1f801070, 0x1f801078))
	{
		switch (vAddr)
		{
		case 0x1f801070:
			//printf("Interrupt Status:       0x%08x\n", interruptStatus);
			return interruptStatus;
			break;
		case 0x1f801074:
			//printf("Interrupt Mask Read:    0x%08x\n", interruptMask);
			return interruptMask;
			break;
		}
	}

	//Check if Reading from Cache Control Register (0xfffe0130)
	if (isInRange(vAddr, 0xfffe0130, 0xfffe0134))
	{
		return cacheReg;
	}

	return psx->rdMem(vAddr, bytes);
}

inline bool CPU::wrMem(uint32_t vAddr, uint32_t& data, uint8_t bytes, bool checkalign)
{
	StatusRegister statusReg;

	statusReg.word = cop0_reg[12];

	//Check if vAddr in unaligned
	if ((bool)(vAddr % bytes) && checkalign)
		exception(static_cast<uint32_t>(exceptionCause::addrerrstore));

	//Check if Cache is Isolated
	if (statusReg.isc)
	{
		//TODO:
		//printf("Write Isolated cache! (addr: %08x bytes: %d)\n",vAddr, bytes);
		//return wrDataCache(vAddr, data, bytes);
		return true;
	}

	//Check if Writing to Data Cache, aka ScratchPad
	if (isInRange(vAddr, 0x1f800000, 0x1f800400))
	{
		return wrDataCache(vAddr, data, bytes);
	}

	//Check if Writing to Interrupt Control Registers (0x1f801070 - 0x1f801074)
	if (isInRange(vAddr, 0x1f801070, 0x1f801078))
	{
		switch (vAddr)
		{
		case 0x1f801070:
			interruptStatus &= data;
			//printf("Interrupt Acknowledge:  0x%08x\n", data);
			return true;
			break;
		case 0x1f801074:
			interruptMask = data;
			//printf("Interrupt Mask Write:   0x%08x\n", data);
			return true;
			break;
		}
	}

	//Check if Writing to Cache Control Register (0xfffe0130)
	if (isInRange(vAddr, 0xfffe0130, 0xfffe0134))
	{
		cacheReg = data;
		iCacheEnabled = (bool)(cacheReg & ICACHE_EN_MASK);
		dCacheEnabled = (bool)(cacheReg & DCACHE_EN1_MASK) && (bool)(cacheReg & DCACHE_EN2_MASK);
		//printf("Cache Register: data: 0x%08x, iCache: %d, dCache: %d, IsC: %d, SwC: %d\n", data, iCacheEnabled, dCacheEnabled, (bool)(cop0_reg[12] & CP0_ISC_MASK), (bool)(cop0_reg[12] & CP0_SWC_MASK));
		return true;
	}

	return psx->wrMem(vAddr, data, bytes);
}

uint32_t CPU::rdDataCache(uint32_t vAddr, uint8_t bytes)
{
	uint32_t data = 0;
	uint32_t addr = vAddr & 0x000003ff;

	if (dCacheEnabled)
	{
		for (int i = 0; i < bytes; i++)
		{
			data += (dCache[addr + i] << (8 * i));
		}
		return data;
	}
	else
	{
		//TODO: Throw Exception?
		return 0;
	}

	return data;
}

bool CPU::wrDataCache(uint32_t vAddr, uint32_t& data, uint8_t bytes)
{
	uint32_t addr = vAddr & 0x000003ff;

	if (dCacheEnabled)
	{
		for (int i = 0; i < bytes; i++)
		{
			dCache[addr + i] = (uint8_t)(data >> (8 * i));
		}

		return true;
	}
	else
	{
		//TODO: Throw Exception?
		return 0;
	}
}

uint32_t CPU::rdInstrCache(uint32_t vAddr, uint8_t bytes)
{
	return uint32_t();
}

bool CPU::wrInstrCache(uint32_t vAddr, uint32_t& data, uint8_t bytes)
{
	return false;
}

//-----------------------------------------------------------------------------------------------------------------------------------
//
// Pipeline Implementation
//
//-----------------------------------------------------------------------------------------------------------------------------------
bool CPU::clock()
{
	bool bResult = true;

	//Stop CPU if DMA is Running
	if (dmaTakeOnBus)
		return true;

	//If Interrupt is triggered skip to next clock cycle
	if (interruptCheck())
		return true;

	//Fetch Instruction from current PC and increment it
	Instruction opcode;
	opcode.word = rdInst(pc);

	//Check if branchDelaySlot is set, in that case we are executing the instruction in the Branch Delay Slot
	//and Program Counter must be set to branch new value, if not just step to the next instruction
	if (branchDelaySlot)
	{
		pc = branchAddress;
		branchDelaySlot = false;
	}
	else
	{
		pc += 4;
	}
	
	//Skip all evaluation if currentOpcode is NOP
	if (opcode.word != 0x00000000)
	{
		//Decode Current Instruction Fields
		currentOpcode.op = opcode.op;
		currentOpcode.funct = opcode.funct;
		currentOpcode.rd = opcode.rd;
		currentOpcode.rt = opcode.rt;
		currentOpcode.rs = opcode.rs;
		currentOpcode.regA = gpr[currentOpcode.rs];
		currentOpcode.regB = gpr[currentOpcode.rt];
		currentOpcode.tgt = opcode.tgt;
		currentOpcode.shamt = opcode.shamt;
		currentOpcode.cofun = opcode.cofun;
		currentOpcode.imm = (uint32_t)(int16_t)opcode.imm; //Sign Extended
		currentOpcode.cop = (bool)opcode.cop;

		//Execute Current Instruction
		if (currentOpcode.op == 0x00)
		{
			bResult = (this->*functSet[currentOpcode.funct].operate)();
			if (!bResult)
				printf("Unimplemented Function %s!\n", functSet[currentOpcode.funct].mnemonic.c_str());
		}
		else
		{
			bResult = (this->*instrSet[currentOpcode.op].operate)();
			if (!bResult)
				printf("Unimplemented Instruction %s!\n", instrSet[currentOpcode.op].mnemonic.c_str());
		}
	}
				
	return bResult;	
}

bool CPU::exception(uint32_t cause)
{
	StatusRegister	statusReg;
	CauseRegister	causeReg;

	printf("EXCEPTION (%d)\n", cause);

	statusReg.word = cop0_reg[12];
	causeReg.word = cop0_reg[13];
	
	//Check if exception occurred in a branch delay slot
	if (branchDelaySlot)
	{
		cop0_reg[14] = pc - 4;		//Set EPC to Jump Instruction if exception occurs in a Branch Delay Slot
		causeReg.bd = true;			//Set BD bit in cop0 CAUSE Register if exception occurs in a Branch Delay Slot
		branchDelaySlot = false;
	}
	else
	{
		cop0_reg[14] = pc;			//Set EPC to Current Instruction if we are not in a Branch Delay Slot
		causeReg.bd = false;		//Set BD bit in cop0 CAUSE Register if exception occurs in a Branch Delay Slot
	}
	
	//Set Exception Code in CAUSE Register
	causeReg.excode = cause;
	
	//Disable Interrupt (shift 2 position left bit [0, 5] for SR)
	statusReg.stk = (statusReg.stk << 2) & 0x3f;
		
	//Jump to exception handler
	if (statusReg.bev)
		pc = 0xbfc00180;
	else
		pc = 0x80000080;

	cop0_reg[13] = causeReg.word;
	cop0_reg[12] = statusReg.word;

	return true;
}

bool CPU::interrupt(uint32_t cause)
{
	//Set I_STAT Interrupt Flag according to the Interrupt Cause
	interruptStatus |= 1UL << cause;

	return true;
}

bool CPU::interruptCheck()
{
	StatusRegister	statusReg;
	CauseRegister	causeReg;

	//Check for any active interrupt
	if (interruptStatus & interruptMask & 0x000007ff)
	{
		//Set cop0r13.bit10
		causeReg.word = cop0_reg[13];
		causeReg.iphw = 0x01;
		cop0_reg[13] = causeReg.word;
	}
	else
	{
		//Reset cop0r13.bit10
		causeReg.word = cop0_reg[13];
		causeReg.iphw = 0x00;
		cop0_reg[13] = causeReg.word;
		return false;
	}

	//Check COP0 Status Register for Interrupt Enable Current
	statusReg.word = cop0_reg[12];
	if (!(bool)statusReg.iec)
		return false;

	//Check COP0 for Pending Interrupt with active Interupt Mask
	//Playstation only use Hardware Interrupt Int0.
	if (statusReg.imhw & causeReg.iphw)
	{
		exception(static_cast<uint32_t>(exceptionCause::interrupt));		
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------------------------------------------------------------
//
// Opcode Implementation
//
//-----------------------------------------------------------------------------------------------------------------------------------
bool CPU::op_bxx()
{
	bool branch = false;

	switch (currentOpcode.rt)
	{
	case 0x00:
		//BLTZ
		branch = ((int32_t)currentOpcode.regA < 0);
		break;
	case 0x01:
		//BGEZ
		branch = ((int32_t)currentOpcode.regA >= 0);
		break;
	case 0x10:
		//BLTZAL
		branch = ((int32_t)currentOpcode.regA < 0);
		gpr[31] = pc + 4; //Return to the istruction after the delay slot
		break;
	case 0x11:
		//BGEZAL
		branch = ((int32_t)currentOpcode.regA >= 0);
		gpr[31] = pc + 4; //Return to the istruction after the delay slot
		break;
	default:
		branch = false;
		printf("Unknown Branch Instruction\n");
		return false;
	}

	//Set branchAddress if Branch is triggered
	//New PC is evaluated starting from PC of the instruction in the delay slot
	if (branch)
	{
		branchAddress = pc + (currentOpcode.imm << 2);
		branchDelaySlot = true;
	}

	return true;
}

bool CPU::op_j()
{
	//Set branchAddress
	//New PC is evaluated starting from PC of the instruction in the delay slot
	branchAddress = (pc & 0xf0000000) + (currentOpcode.tgt << 2);
	branchDelaySlot = true;

	return true;
}

bool CPU::op_jal()
{
	//Set branchAddress
	//New PC is evaluated starting from PC of the instruction in the delay slot
	branchAddress = (pc & 0xf0000000) + (currentOpcode.tgt << 2);
	branchDelaySlot = true;
	gpr[31] = pc + 4;	//Return to the istruction after the delay slot
 
	return true;
}

bool CPU::op_beq()
{
	//Set branchAddress if Branch is triggered
	//New PC is evaluated starting from PC of the instruction in the delay slot
	if (currentOpcode.regA == currentOpcode.regB)
	{
		branchAddress = pc + (currentOpcode.imm << 2);
		branchDelaySlot = true;
	}

	return true;
}

bool CPU::op_bne()
{
	//Set branchAddress if Branch is triggered
	//New PC is evaluated starting from PC of the instruction in the delay slot
	if (currentOpcode.regA != currentOpcode.regB)
	{
		branchAddress = pc + (currentOpcode.imm << 2);
		branchDelaySlot = true;
	}

	return true;
}

bool CPU::op_blez()
{
	//Set branchAddress if Branch is triggered
	//New PC is evaluated starting from PC of the instruction in the delay slot
	if ((int32_t)currentOpcode.regA <= 0)
	{
		branchAddress = pc + (currentOpcode.imm << 2);
		branchDelaySlot = true;
	}

	return true;
}

bool CPU::op_bgtz()
{
	//Set branchAddress if Branch is triggered
	//New PC is evaluated starting from PC of the instruction in the delay slot
	if ((int32_t)currentOpcode.regA > 0)
	{
		branchAddress = pc + (currentOpcode.imm << 2);
		branchDelaySlot = true;
	}

	return true;
}

bool CPU::op_addi()
{
	//Check for integer overflow
	bool overflow = isOverflow((int32_t)currentOpcode.regA, (int32_t)currentOpcode.imm);
	
	if (!overflow && currentOpcode.rt != 0)
		gpr[currentOpcode.rt] = currentOpcode.regA + currentOpcode.imm;
			
	if (overflow)
	{
		exception(static_cast<uint32_t>(exceptionCause::overflow));
		printf("addi throw Integer Overflow Exception!\n");
	}

	return true;
}

bool CPU::op_addiu()
{
	if (currentOpcode.rt != 0)
		gpr[currentOpcode.rt] = currentOpcode.regA + currentOpcode.imm;

	return true;
}

bool CPU::op_slti()
{
	if (currentOpcode.rt != 0)
		gpr[currentOpcode.rt] = ((int32_t)currentOpcode.regA < (int32_t)currentOpcode.imm) ? 0x00000001 : 0x00000000;

	return true;
}

bool CPU::op_sltiu()
{
	if (currentOpcode.rt != 0)
		gpr[currentOpcode.rt] = ((uint32_t)currentOpcode.regA < (uint32_t)currentOpcode.imm) ? 0x00000001 : 0x00000000;
	
	return true;
}

bool CPU::op_andi()
{
	if (currentOpcode.rt != 0)
		gpr[currentOpcode.rt] = currentOpcode.regA & (0x0000ffff & currentOpcode.imm); //imm is zero extended

	return true;
}

bool CPU::op_ori()
{
	if (currentOpcode.rt != 0)
		gpr[currentOpcode.rt] = currentOpcode.regA | (0x0000ffff & currentOpcode.imm); //imm is zero extended

	return true;
}

bool CPU::op_xori()
{
	if (currentOpcode.rt != 0)
		gpr[currentOpcode.rt] = currentOpcode.regA ^ (0x0000ffff & currentOpcode.imm); //imm is zero extended
		
	return true;
}

bool CPU::op_lui()
{
	if (currentOpcode.rt != 0)
		gpr[currentOpcode.rt] = currentOpcode.imm << 16;

	return true;
}

bool CPU::op_cop0()
{
	if (currentOpcode.cop)
	{
		switch (currentOpcode.funct)
		{
		case 0x01:	//tlbr
			exception(static_cast<uint32_t>(exceptionCause::copunusable));
			printf("Error! - tlbr unsupported\n");
			break;
		case 0x02:	//tlbwi
			exception(static_cast<uint32_t>(exceptionCause::copunusable));
			printf("Error! - tlbwi unsupported\n");
			break;
		case 0x06:	//tlbwr
			exception(static_cast<uint32_t>(exceptionCause::copunusable));
			printf("Error! - tlbwr unsupported\n");
			break;
		case 0x08:	//tlbp
			exception(static_cast<uint32_t>(exceptionCause::copunusable));
			printf("Error! - tlbp unsupported\n");
			break;
		case 0x10:	//rfe
			//Restore Interrupt Status (shift 2 position right bit [0, 5] for SR, bit 5 and 4 remain untouched)
			StatusRegister statusReg;
			statusReg.word = cop0_reg[12];
			statusReg.stk = ((statusReg.stk >> 2) & 0x0f) | (statusReg.stk & 0x30);
			cop0_reg[12] = statusReg.word;
			break;
		}
	}
	else
	{
		switch (currentOpcode.rs)
		{
		case 0x00:	//mfc0 rt, rd
			if (currentOpcode.rt != 0)
			{
				gpr[currentOpcode.rt] = cop0_reg[currentOpcode.rd];
				//printf("Move (0x%08x) from CPR[0, %d] to GPR[%d]\n", cop0_reg[currentOpcode.rd], currentOpcode.rd, currentOpcode.rt);
			}
				
			break;
		case 0x02:	//cfc0 rt, rd
			exception(static_cast<uint32_t>(exceptionCause::copunusable));
			printf("Error! - cfc0 is unsupported\n");
			break;
		case 0x04:	//mtc0 rt, rd
			cop0_reg[currentOpcode.rd] = currentOpcode.regB;
			//printf("Move (0x%08x) from GPR[%d] to CPR[0, %d]\n", currentOpcode.regB, currentOpcode.rt, currentOpcode.rd);
			break;
		case 0x06:	//ctc0 rt, rd
			exception(static_cast<uint32_t>(exceptionCause::copunusable));
			printf("Error! - ctc0 is unsupported\n");
			break;
		}
	}

	return true;
}

bool CPU::op_cop1()
{
	exception(static_cast<uint32_t>(exceptionCause::copunusable));

	return true;
}

bool CPU::op_cop2()
{
	if (currentOpcode.cop)
	{
		printf("Error! - cop2 not implemented yet!\n");
	}
	else
	{
		switch (currentOpcode.rs)
		{
		case 0x00:	//mfc2 rt, rd
			printf("Error! - mfc2 not implemented yet!\n");
			break;
		case 0x02:	//cfc2 rt, rd
			printf("Error! - cfc2 not implemented yet!\n");
			break;
		case 0x04:	//mtc2 rt, rd
			printf("Error! - mtc2 not implemented yet!\n");
			break;
		case 0x06:	//ctc2 rt, rd
			printf("Error! - ctc2 not implemented yet!\n");
			break;
		}
	}

	return true;
}

bool CPU::op_cop3()
{
	exception(static_cast<uint32_t>(exceptionCause::copunusable));

	return true;
}

bool CPU::op_lb()
{
	if (currentOpcode.rt != 0)
		gpr[currentOpcode.rt] = (uint32_t)(int8_t)rdMem(currentOpcode.regA + currentOpcode.imm, 1);

	return true;
}

bool CPU::op_lh()
{
	if (currentOpcode.rt != 0)
		gpr[currentOpcode.rt] = (uint32_t)(int16_t)rdMem(currentOpcode.regA + currentOpcode.imm, 2);

	return true;
}

bool CPU::op_lwl()
{
	//Unaligned reading from memory is approximated with a 4 byte reading from LSB
	//for both LWL and LWR. Both act like LW but starting from a potentially
	//unaligned address.
	if (currentOpcode.rt != 0)
		gpr[currentOpcode.rt] = rdMem(currentOpcode.regA + currentOpcode.imm - 0x03, 4, false); //treated as lw but startin at the LSB

	//TODO: sta roba non funziona! sul registro destinazione devo sovrapporre le due letture LWL e LWR. Qui le sovrascrivo	
	return true;
}

bool CPU::op_lw()
{
	if (currentOpcode.rt != 0)
		gpr[currentOpcode.rt] = rdMem(currentOpcode.regA + currentOpcode.imm, 4);

	return true;
}

bool CPU::op_lbu()
{
	if (currentOpcode.rt != 0)
		gpr[currentOpcode.rt] = rdMem(currentOpcode.regA + currentOpcode.imm, 1);

	return true;
}

bool CPU::op_lhu()
{
	if (currentOpcode.rt != 0)
		gpr[currentOpcode.rt] = rdMem(currentOpcode.regA + currentOpcode.imm, 2);

	return true;
}

bool CPU::op_lwr()
{
	//Checked 25/08/2021
	//Unaligned reading from memory is approximated with a 4 byte reading from LSB
	//for both LWL and LWR. Both act like LW but starting from a potentially
	//unaligned address.
 	if (currentOpcode.rt != 0)
		gpr[currentOpcode.rt] = rdMem(currentOpcode.regA + currentOpcode.imm, 4, false); //treated as lw but startin at the LSB

	//TODO: sta roba non funziona! sul registro destinazione devo sovrapporre le due letture LWL e LWR. Qui le sovrascrivo
	return true;
}

bool CPU::op_sb()
{
	//Checked 05/07/2021
	wrMem(currentOpcode.regA + currentOpcode.imm, currentOpcode.regB, 1);

	return true;
}

bool CPU::op_sh()
{
	wrMem(currentOpcode.regA + currentOpcode.imm, currentOpcode.regB, 2);
	
	return true;
}

bool CPU::op_swl()
{
	//Unaligned writing to memory is approximated with a 4 byte writing from LSB
	//for both SWL and SWR. Both act like SW but starting from a potentially
	//unaligned address.
	wrMem(currentOpcode.regA + currentOpcode.imm - 0x03, currentOpcode.regB, 4, false);

	return true;
}

bool CPU::op_sw()
{
	wrMem(currentOpcode.regA + currentOpcode.imm, currentOpcode.regB, 4);

	return true;
}

bool CPU::op_swr()
{
	//Unaligned writing to memory is approximated with a 4 byte writing from LSB
	//for both SWL and SWR. Both act like SW but starting from a potentially
	//unaligned address.
	wrMem(currentOpcode.regA + currentOpcode.imm, currentOpcode.regB, 4, false);
		
	return true;
}

bool CPU::op_lwc0()
{
	//Checked 16/07/2021

	exception(static_cast<uint32_t>(exceptionCause::copunusable));

	return true;
}

bool CPU::op_lwc1()
{
	//Checked 16/07/2021

	exception(static_cast<uint32_t>(exceptionCause::copunusable));

	return true;
}

bool CPU::op_lwc2()
{
	return false;
}

bool CPU::op_lwc3()
{
	//Checked 16/07/2021

	exception(static_cast<uint32_t>(exceptionCause::copunusable));

	return true;
}

bool CPU::op_swc0()
{
	//Checked 16/07/2021

	exception(static_cast<uint32_t>(exceptionCause::copunusable));

	return true;
}

bool CPU::op_swc1()
{
	//Checked 16/07/2021

	exception(static_cast<uint32_t>(exceptionCause::copunusable));

	return true;
}

bool CPU::op_swc2()
{
	return false;
}

bool CPU::op_swc3()
{
	//Checked 16/07/2021

	exception(static_cast<uint32_t>(exceptionCause::copunusable));

	return true;
}

bool CPU::op_sll()
{
	if (currentOpcode.rd != 0)
		gpr[currentOpcode.rd] = currentOpcode.regB << currentOpcode.shamt;

	return true;
}

bool CPU::op_srl()
{
	if (currentOpcode.rd != 0)
		gpr[currentOpcode.rd] = currentOpcode.regB >> currentOpcode.shamt;

	return true;
}

bool CPU::op_sra()
{
	if (currentOpcode.rd != 0)
		gpr[currentOpcode.rd] = (int32_t)currentOpcode.regB >> currentOpcode.shamt;

	return true;
}

bool CPU::op_sllv()
{
	if (currentOpcode.rd != 0)
		gpr[currentOpcode.rd] = currentOpcode.regB << (currentOpcode.regA & 0x0000001f);

	return true;
}

bool CPU::op_srlv()
{
	if (currentOpcode.rd != 0)
		gpr[currentOpcode.rd] = currentOpcode.regB >> (currentOpcode.regA & 0x0000001f);

	return true;
}

bool CPU::op_srav()
{
	if (currentOpcode.rd != 0)
		gpr[currentOpcode.rd] = (int32_t)currentOpcode.regB >> (currentOpcode.regA & 0x0000001f);

	return true;
}

bool CPU::op_jr()
{
	//Foreward PC to fetch stage
	branchAddress = currentOpcode.regA;
	branchDelaySlot = true;

	return true;
}

bool CPU::op_jalr()
{
	//Foreward PC to fetch stage
	branchAddress = currentOpcode.regA;
	branchDelaySlot = true;
	gpr[31] = pc + 4;

	return true;
}

bool CPU::op_syscall()
{
	exception(static_cast<uint32_t>(exceptionCause::syscall));

	return true;
}

bool CPU::op_break()
{
	exception(static_cast<uint32_t>(exceptionCause::breakpoint));

	return true;
}

bool CPU::op_mfhi()
{
	if (currentOpcode.rd != 0)
		gpr[currentOpcode.rd] = hi;
	
	return true;
}

bool CPU::op_mthi()
{
	hi = currentOpcode.regA;

	return true;
}

bool CPU::op_mflo()
{
	if (currentOpcode.rd != 0)
		gpr[currentOpcode.rd] = lo;

	return true;
}

bool CPU::op_mtlo()
{
	lo = currentOpcode.regA;

	return true;
}

bool CPU::op_mult()
{
	int64_t a, b, r;

	a = (int64_t)(int32_t)currentOpcode.regA;
	b = (int64_t)(int32_t)currentOpcode.regB;
	r = a * b;
	hi = (uint32_t)(r >> 32);
	lo = (uint32_t)r;

	return true;
}

bool CPU::op_multu()
{
	uint64_t a, b, r;

	a = (uint64_t)currentOpcode.regA;
	b = (uint64_t)currentOpcode.regB;
	r = a * b;
	hi = (uint32_t)(r >> 32);
	lo = (uint32_t)r;

	return true;
}

bool CPU::op_div()
{
	int32_t n, d;

	n = currentOpcode.regA;
	d = currentOpcode.regB;

	//Special Cases
	if (d == 0)
	{
		hi = n;
		if (n >= 0)
			lo = 0xffffffff;
		else
			lo = 0x00000001;
	}
	else if (n == 0x80000000 && d == 0xffffffff)
	{
		hi = 0x00000000;
		lo = 0x80000000;
	}
	else
	{
		hi = n % d;
		lo = n / d;
	}

	return true;
}

bool CPU::op_divu()
{
	int32_t n, d;

	n = currentOpcode.regA;
	d = currentOpcode.regB;

	//Special Cases
	if (d == 0)
	{
		hi = n;
		lo = 0xffffffff;	
	}
	else
	{
		hi = n % d;
		lo = n / d;
	}

	return true;
}

bool CPU::op_add()
{
	 
	//Check for integer overflow
	bool overflow = isOverflow((int32_t)currentOpcode.regA, (int32_t)currentOpcode.regB);

	if (!overflow && currentOpcode.rd != 0)
		gpr[currentOpcode.rd] = (int32_t)currentOpcode.regA + (int32_t)currentOpcode.regB;
	
	if (overflow)
		exception(static_cast<uint32_t>(exceptionCause::overflow));

	return true;
}

bool CPU::op_addu()
{
	if (currentOpcode.rd != 0)
		gpr[currentOpcode.rd] = (uint32_t)currentOpcode.regA + (uint32_t)currentOpcode.regB;

	return true;
}

bool CPU::op_sub()
{
	//Check for integer overflow
	bool overflow = isOverflow((int32_t)currentOpcode.regA, (int32_t)currentOpcode.regB);

	if (!overflow && currentOpcode.rd != 0)
		gpr[currentOpcode.rd] = (int32_t)currentOpcode.regA - (int32_t)currentOpcode.regB;

	if (overflow)
		exception(static_cast<uint32_t>(exceptionCause::overflow));
	
	return true;
}

bool CPU::op_subu()
{
	if (currentOpcode.rd != 0)
		gpr[currentOpcode.rd] = (uint32_t)currentOpcode.regA - (uint32_t)currentOpcode.regB;

	return true;
}

bool CPU::op_and()
{
	if (currentOpcode.rd != 0)
		gpr[currentOpcode.rd] = currentOpcode.regA & currentOpcode.regB;

	return true;
}

bool CPU::op_or()
{
	if (currentOpcode.rd != 0)
		gpr[currentOpcode.rd] = currentOpcode.regA | currentOpcode.regB;

	return true;
}

bool CPU::op_xor()
{
	if (currentOpcode.rd != 0)
		gpr[currentOpcode.rd] = currentOpcode.regA ^ currentOpcode.regB;

	return true;
}

bool CPU::op_nor()
{
	if (currentOpcode.rd != 0)
		gpr[currentOpcode.rd] = ~(currentOpcode.regA | currentOpcode.regB);

	return true;
}

bool CPU::op_slt()
{
	if (currentOpcode.rd != 0)
		gpr[currentOpcode.rd] = ((int32_t)currentOpcode.regA < (int32_t)currentOpcode.regB) ? 0x00000001 : 0x00000000;

	return true;
}

bool CPU::op_sltu()
{
	if (currentOpcode.rd != 0)
		gpr[currentOpcode.rd] = ((uint32_t)currentOpcode.regA < (uint32_t)currentOpcode.regB) ? 0x00000001 : 0x00000000;

	return true;
}

bool CPU::op_unknown()
{
	exception(static_cast<uint32_t>(exceptionCause::resinst));
	
	return true;
}

