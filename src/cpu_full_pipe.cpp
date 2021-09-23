#include "cpu_full_pipe.h"
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
	memset(&ifidReg, 0x00000000, sizeof(ifidBuffer));
	memset(&idexReg, 0x00000000, sizeof(idexBuffer));
	memset(&exmemReg, 0x00000000, sizeof(exmemBuffer));
	memset(&memwbReg, 0x00000000, sizeof(memwbBuffer));
	stallPipeline = false;
	dmaTakeOnBus = false;

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
	memset(&ifidReg, 0x00000000, sizeof(ifidBuffer));
	memset(&idexReg, 0x00000000, sizeof(idexBuffer));
	memset(&exmemReg, 0x00000000, sizeof(exmemBuffer));
	memset(&memwbReg, 0x00000000, sizeof(memwbBuffer));
	stallPipeline = false;
	dmaTakeOnBus = false;

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
uint32_t CPU::rdInst(uint32_t vAddr, uint8_t bytes)
{
	//Check if PC in unaligned
	if ((bool)(vAddr % bytes))
		exception(static_cast<uint32_t>(exceptionCause::addrerrload));

	//TODO: Instruction Cache Management

	return psx->rdMem(vAddr, bytes);
}

uint32_t CPU::rdMem(uint32_t vAddr, uint8_t bytes, bool checkalign)
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
			//printf("Interrupt Status:		0x%08x\n", interruptStatus);
			return interruptStatus;
			break;
		case 0x1f801074:
			//printf("Interrupt Mask Read:	0x%08x\n", interruptMask);
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

bool CPU::wrMem(uint32_t vAddr, uint32_t& data, uint8_t bytes, bool checkalign)
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
			//printf("Interrupt Acknowledge:	0x%08x\n", data);
			return true;
			break;
		case 0x1f801074:
			interruptMask = data;
			//printf("Interrupt Mask Write:	    0x%08x\n", data);
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
	//Stop CPU if DMA is Running (TODO: Check if Clock Correct)
	if (dmaTakeOnBus)
		return true;

	//Go Thru all five stages of the pipeline
	writeback();
	memoryaccess();
	execute();
	decode();
	fetch();

	//Check for any needed Forwarding or Pipeline Stall to be applied on the next Clock Cycle.
	forwardCheck();

	//Check Interrupt Status
	interruptCheck();
			
	return true;
}

bool CPU::fetch()
{
	//If Pipeline is Stalled exit immediatly
	if (stallPipeline)
		return false;
		
	ifidReg.instr = rdInst(pc);
	ifidReg.pc = pc;

	pc = pc + 4;
			
	return true;
}

bool CPU::decode()
{
	Instruction opcode;

	//If Pipeline is Stalled exit immediatly
	if (stallPipeline)
		return false;

	//Pass Control Signals to next Pipeline Register
	idexReg.pc = ifidReg.pc;
	idexReg.branchDelaySlot = ifidReg.branchDelaySlot;
	ifidReg.branchDelaySlot = false;
	
	opcode.word = ifidReg.instr;
	idexReg.op = opcode.op;
	idexReg.funct = opcode.funct;
	idexReg.rd = opcode.rd;
	idexReg.rt = opcode.rt;
	idexReg.rs = opcode.rs;
	idexReg.regA = gpr[idexReg.rs];
	idexReg.regB = gpr[idexReg.rt];
	idexReg.tgt = opcode.tgt;
	idexReg.shamt = opcode.shamt;
	idexReg.cofun = opcode.cofun;
	idexReg.imm = (uint32_t)(int16_t)opcode.imm; //Sign Extended
	idexReg.cop = (bool)opcode.cop;
			
	return true;
}

bool CPU::execute()
{
	bool bResult;

	//If Pipeline is Stalled insert a NOP
	if (stallPipeline)
	{
		memset(&exmemReg, 0x00000000, sizeof(exmemBuffer));
		return false;
	}
	
	//Pass Control Signals to next Pipeline Register
	exmemReg.pc = idexReg.pc;

	if (idexReg.op == 0x00)
	{
		bResult = (this->*functSet[idexReg.funct].operate)();
		if (!bResult)
			printf("Unimplemented Function %s!\n", functSet[idexReg.funct].mnemonic.c_str());
	}
	else
	{
		bResult = (this->*instrSet[idexReg.op].operate)();
		if (!bResult)
			printf("Unimplemented Instruction %s!\n", instrSet[idexReg.op].mnemonic.c_str());
	}

	return bResult;
}

bool CPU::memoryaccess()
{
	//Pass Control Signals to next Pipeline Register
	memwbReg.aluRes = exmemReg.aluRes;
	memwbReg.rd = exmemReg.rd;
	memwbReg.memToReg = exmemReg.memToReg;
	memwbReg.aluToReg = exmemReg.aluToReg;
	memwbReg.pc = exmemReg.pc;
	
	//Read From Memory with no Sign Extension
	if (exmemReg.readMem)
	{
		memwbReg.memData = rdMem(exmemReg.aluRes, exmemReg.bytes);
		return true;
	}

	//Write to Memory
	if (exmemReg.writeMem)
	{
		wrMem(exmemReg.aluRes, exmemReg.regB, exmemReg.bytes);
		return true;
	}

	//Read From Memory with Sign Extension
	if (exmemReg.readMemSignExt)
	{
		switch (exmemReg.bytes)
		{
		case 1:
			memwbReg.memData = (uint32_t)(int8_t)rdMem(exmemReg.aluRes, 1);
			break;
		case 2:
			memwbReg.memData = (uint32_t)(int16_t)rdMem(exmemReg.aluRes, 2);
			break;
		default:
			printf("ERROR - Reading from memory more than 2 bytes with Sign Extension!\n");
		}
		return true;
	}

	//Read From Memory with unaligned address
	if (exmemReg.readMemUnaligned)
	{
		memwbReg.memData = rdMem(exmemReg.aluRes, exmemReg.bytes, false);
		return true;
	}

	//Write to Memory with unaligned address
	if (exmemReg.writeMemUnaligned)
	{
		wrMem(exmemReg.aluRes, exmemReg.regB, exmemReg.bytes, false);
		return true;
	}
			
	return false;
}

bool CPU::writeback()
{
	//Check if trying to write on $0 and exit immediately
	if (memwbReg.rd == 0)
		return true;

	//Write to Register
	if (memwbReg.memToReg)
		gpr[memwbReg.rd] = memwbReg.memData; //Write from Memory
	else if (memwbReg.aluToReg)
		gpr[memwbReg.rd] = memwbReg.aluRes;	//Write ALU Result
	
	return true;
}

bool CPU::forwardCheck()
{
	//Order is important, Check in mem/wb register before ex/mem register in order to catch the latest
	//value in case of two consecutive instruction that change the same register whose value is used
	// by the third instruction
	 
	//Check if Forewarding is needed beetwen WriteBack and Execute
	if (memwbReg.aluToReg && memwbReg.rd != 0)
	{
		if (idexReg.rs == memwbReg.rd)
		{
			idexReg.regA = memwbReg.aluRes; //Foreward to EXE the new value for rs from R-Type Instruction
		}
		else if (idexReg.rt == memwbReg.rd)
		{
			idexReg.regB = memwbReg.aluRes; //Foreward to EXE the new value for rt from R-Type Instruction
		}
	}
	else if (memwbReg.memToReg && memwbReg.rd != 0)
	{
		if (idexReg.rs == memwbReg.rd)
		{
			idexReg.regA = memwbReg.memData; //Foreward to EXE the new value for rs from I-Type Instruction
		}
		else if (idexReg.rt == memwbReg.rd)
		{
			idexReg.regB = memwbReg.memData; //Foreward to EXE the new value for rt from I-Type Instruction
		}
	}

	//Check if Forewarding is needed beetwen MemoryAccess and Execute
	stallPipeline = false;
	if (exmemReg.aluToReg && exmemReg.rd != 0)
	{	
		if (idexReg.rs == exmemReg.rd)
		{
			idexReg.regA = exmemReg.aluRes;	//Foreward to EXE the new value for rs from R-Type Instruction
		}
		else if (idexReg.rt == exmemReg.rd)
		{
			idexReg.regB = exmemReg.aluRes; //Foreward to EXE the new value for rt from R-Type Instruction
		}
	}
	else if (exmemReg.memToReg && exmemReg.rd != 0)
	{
		if (idexReg.rs == exmemReg.rd)
		{
			stallPipeline = true;			//Stall - EXE needs the new value for rs from I-Type Instruction, but it has to be read from memory first
		}
		else if (idexReg.rt == exmemReg.rd)
		{
			stallPipeline = true;			//Stall - EXE needs the new value for rt from I-Type Instruction, but it has to be read from memory first
		}
	}

	return true;
}

bool CPU::exception(uint32_t cause)
{
	StatusRegister	statusReg;
	CauseRegister	causeReg;

	statusReg.word = cop0_reg[12];
	causeReg.word = cop0_reg[13];
	
	//Check if exception occurred in a branch delay slot
	if (idexReg.branchDelaySlot)
	{
		cop0_reg[14] = exmemReg.pc; //Set EPC to Jump Instruction if exception occurs in a Branch Delay Slot
		causeReg.bd = true;			//Set BD bit in cop0 CAUSE Register if exception occurs in a Branch Delay Slot
	}
	else
	{
		cop0_reg[14] = idexReg.pc;  //Set EPC to Current Instruction if we are not in a Branch Delay Slot
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

	//Exceptions don't have a Delay Slot. Emulating this behaviour putting NOP on the IF/ID Register
	memset(&ifidReg, 0x00000000, sizeof(ifidBuffer));

	cop0_reg[13] = causeReg.word;
	cop0_reg[12] = statusReg.word;

	return true;
}

bool CPU::interrupt(uint32_t hwInterrupt)
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
	//Checked 05/07/2021

	bool branch = false;

	switch (idexReg.rt)
	{
	case 0x00:
		//BLTZ
		branch = ((int32_t)idexReg.regA < 0);
		exmemReg.rd = 0;
		exmemReg.aluToReg = false;
		exmemReg.aluRes = 0;
		break;
	case 0x01:
		//BGEZ
		branch = ((int32_t)idexReg.regA >= 0);
		exmemReg.rd = 0;
		exmemReg.aluToReg = false;
		exmemReg.aluRes = 0;
		break;
	case 0x10:
		//BLTZAL
		branch = ((int32_t)idexReg.regA < 0);
		exmemReg.rd = 31;
		exmemReg.aluToReg = true;
		exmemReg.aluRes = idexReg.pc + 8;	//Return to the istruction after the delay slot
		break;
	case 0x11:
		//BGEZAL
		branch = ((int32_t)idexReg.regA >= 0);
		exmemReg.rd = 31;
		exmemReg.aluToReg = true;
		exmemReg.aluRes = idexReg.pc + 8;	//Return to the istruction after the delay slot
		break;
	default:
		branch = false;
		exmemReg.rd = 0;
		exmemReg.aluToReg = false;
		exmemReg.aluRes = 0;
		printf("Unknown Branch Instruction\n");
		return false;
	}

	//Set new PC value to fetch stage if Branch is triggered
	//New PC is evaluated starting from PC of the instruction in the delay slot
	if (branch)
	{
		pc = ifidReg.pc + (idexReg.imm << 2);
		ifidReg.branchDelaySlot = true;
	}
		
	exmemReg.memToReg = false;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;
	return true;
}

bool CPU::op_j()
{
	//Checked 05/07/2021
	// 
	//Set new PC value to fetch stage if Branch is triggered
	//New PC is evaluated starting from PC of the instruction in the delay slot
	pc = (ifidReg.pc & 0xf0000000) | (idexReg.tgt << 2);
	ifidReg.branchDelaySlot = true;
	
	exmemReg.rd = 0;
	exmemReg.aluRes = 0;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = false;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_jal()
{
	//Checked 05/07/2021
	// 
	//Set new PC value to fetch stage if Branch is triggered
	//New PC is evaluated starting from PC of the instruction in the delay slot
   	pc = (ifidReg.pc & 0xf0000000) + (idexReg.tgt << 2);
	ifidReg.branchDelaySlot = true;

	exmemReg.rd = 31;
	exmemReg.aluRes = idexReg.pc + 8; //Return to the istruction after the delay slot
	exmemReg.memToReg = false;
	exmemReg.aluToReg = true;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_beq()
{
	//Checked 05/07/2021
	// 
	//Set new PC value to fetch stage if Branch is triggered
	//New PC is evaluated starting from PC of the instruction in the delay slot
	if (idexReg.regA == idexReg.regB)
	{
		pc = ifidReg.pc + (idexReg.imm << 2);
		ifidReg.branchDelaySlot = true;
	}
		
	exmemReg.rd = 0;
	exmemReg.aluRes = 0;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = false;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_bne()
{
	//Checked 05/07/2021
	// 
	//Set new PC value to fetch stage if Branch is triggered
	//New PC is evaluated starting from PC of the instruction in the delay slot
	if (idexReg.regA != idexReg.regB)
	{
		pc = ifidReg.pc + (idexReg.imm << 2);
		ifidReg.branchDelaySlot = true;
	}
		
	exmemReg.rd = 0;
	exmemReg.aluRes = 0;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = false;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_blez()
{
	//Checked 05/07/2021
	// 
	//Set new PC value to fetch stage if Branch is triggered
	//New PC is evaluated starting from PC of the instruction in the delay slot
	if ((int32_t)idexReg.regA <= 0)
	{
		pc = ifidReg.pc + (idexReg.imm << 2);
		ifidReg.branchDelaySlot = true;
	}
		
	exmemReg.rd = 0;
	exmemReg.aluRes = 0;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = false;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_bgtz()
{
	//Checked 05/07/2021
	// 
	//Set new PC value to fetch stage if Branch is triggered
	//New PC is evaluated starting from PC of the instruction in the delay slot
	if ((int32_t)idexReg.regA > 0)
	{
		pc = ifidReg.pc + (idexReg.imm << 2);
		ifidReg.branchDelaySlot = true;
	}
		
	exmemReg.rd = 0;
	exmemReg.aluRes = 0;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = false;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_addi()
{
	//Checked 05/07/2021
	// 
	//Check for integer overflow
	bool overflow = isOverflow((int32_t)idexReg.regA, (int32_t)idexReg.imm);
	
	exmemReg.rd = idexReg.rt;
	exmemReg.aluRes =  idexReg.regA + idexReg.imm;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = !overflow; //Modify rd only if not overflow
	exmemReg.regB = idexReg.regB;       
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;
		
	if (overflow)
	{
		exception(static_cast<uint32_t>(exceptionCause::overflow));
		printf("addi throw Integer Overflow Exception!\n");
	}

	return true;
}

bool CPU::op_addiu()
{
	//Checked 05/07/2021

	exmemReg.rd = idexReg.rt;
	exmemReg.aluRes = idexReg.regA + idexReg.imm;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = true;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_slti()
{
	//Checked 05/07/2021

	exmemReg.rd = idexReg.rt;
	exmemReg.aluRes = ((int32_t)idexReg.regA < (int32_t)idexReg.imm) ? 0x00000001 : 0x00000000;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = true;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_sltiu()
{
	//Checked 05/07/2021

	exmemReg.rd = idexReg.rt;
	exmemReg.aluRes = ((uint32_t)idexReg.regA < (uint32_t)idexReg.imm) ? 0x00000001 : 0x00000000;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = true;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_andi()
{
	//Checked 05/07/2021

	exmemReg.rd = idexReg.rt;
	exmemReg.aluRes = idexReg.regA & (0x0000ffff & idexReg.imm); //imm is zero extended
	exmemReg.memToReg = false;
	exmemReg.aluToReg = true;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_ori()
{
	//Checked 05/07/2021

	exmemReg.rd = idexReg.rt;
	exmemReg.aluRes = idexReg.regA | (0x0000ffff & idexReg.imm); //imm is zero extended
	exmemReg.memToReg = false;
	exmemReg.aluToReg = true;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_xori()
{
	//Checked 16/07/2021

	exmemReg.rd = idexReg.rt;
	exmemReg.aluRes = idexReg.regA ^ (0x0000ffff & idexReg.imm); //imm is zero extended
	exmemReg.memToReg = false;
	exmemReg.aluToReg = true;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_lui()
{
	//Checked 05/07/2021

	exmemReg.rd = idexReg.rt;
	exmemReg.aluRes = idexReg.imm << 16;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = true;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_cop0()
{
	//Checked 17/07/2021

	exmemReg.rd = 0;
	exmemReg.aluRes = 0;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = false;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	if (idexReg.cop)
	{
		switch (idexReg.funct)
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
		switch (idexReg.rs)
		{
		case 0x00:	//mfc0 rt, rd
			exmemReg.rd = idexReg.rt;
			exmemReg.aluRes = cop0_reg[idexReg.rd];
			exmemReg.aluToReg = true;
			break;
		case 0x02:	//cfc0 rt, rd
			exception(static_cast<uint32_t>(exceptionCause::copunusable));
			printf("Error! - cfc0 is unsupported\n");
			break;
		case 0x04:	//mtc0 rt, rd
			cop0_reg[idexReg.rd] = idexReg.regB; //Check if clock correct
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
	//Checked 16/07/2021
	
	exception(static_cast<uint32_t>(exceptionCause::copunusable));

	return true;
}

bool CPU::op_cop2()
{
	exmemReg.rd = 0;
	exmemReg.aluRes = 0;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = false;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	if (idexReg.cop)
	{
		printf("Error! - cop2 not implemented yet!\n");
	}
	else
	{
		switch (idexReg.rs)
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
	//Checked 16/07/2021

	exception(static_cast<uint32_t>(exceptionCause::copunusable));

	return true;
}

bool CPU::op_lb()
{
	//Checked 05/07/2021

	exmemReg.rd = idexReg.rt;
	exmemReg.aluRes = idexReg.regA + idexReg.imm;
	exmemReg.memToReg = true;
	exmemReg.aluToReg = false;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = true;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 1;

	return true;
}

bool CPU::op_lh()
{
	//Checked 17/07/2021

	exmemReg.rd = idexReg.rt;
	exmemReg.aluRes = idexReg.regA + idexReg.imm;
	exmemReg.memToReg = true;
	exmemReg.aluToReg = false;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = true;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 2;

	return true;
}

bool CPU::op_lwl()
{
	//Checked 25/08/2021
	//Unaligned reading from memory is approximated with a 4 byte reading from LSB
	//for both LWL and LWR. Both act like LW but starting from a potentially
	//unaligned address.

	exmemReg.rd = idexReg.rt;
	exmemReg.aluRes = idexReg.regA + idexReg.imm - 0x03;	//treated as lw but startin at the LSB
	exmemReg.memToReg = true;
	exmemReg.aluToReg = false;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = true;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 4;

	return true;
}

bool CPU::op_lw()
{
	//Checked 05/07/2021

	exmemReg.rd = idexReg.rt;
	exmemReg.aluRes = idexReg.regA + idexReg.imm;
	exmemReg.memToReg = true;
	exmemReg.aluToReg = false;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = true;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 4;

	return true;
}

bool CPU::op_lbu()
{
	//Checked 05/07/2021

	exmemReg.rd = idexReg.rt;
	exmemReg.aluRes = idexReg.regA + idexReg.imm;
	exmemReg.memToReg = true;
	exmemReg.aluToReg = false;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = true;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 1;

	return true;
}

bool CPU::op_lhu()
{
	//Checked 17/07/2021

	exmemReg.rd = idexReg.rt;
	exmemReg.aluRes = idexReg.regA + idexReg.imm;
	exmemReg.memToReg = true;
	exmemReg.aluToReg = false;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = true;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 2;

	return true;
}

bool CPU::op_lwr()
{
	//Checked 25/08/2021
	//Unaligned reading from memory is approximated with a 4 byte reading from LSB
	//for both LWL and LWR. Both act like LW but starting from a potentially
	//unaligned address.

	exmemReg.rd = idexReg.rt;
	exmemReg.aluRes = idexReg.regA + idexReg.imm;	//treated as lw
	exmemReg.memToReg = true;
	exmemReg.aluToReg = false;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = true;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 4;

	return true;
}

bool CPU::op_sb()
{
	//Checked 05/07/2021

	exmemReg.rd = 0;
	exmemReg.aluRes = idexReg.regA + idexReg.imm;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = false;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = true;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 1;

	return true;
}

bool CPU::op_sh()
{
	//Checked 05/07/2021

	exmemReg.rd = 0;
	exmemReg.aluRes = idexReg.regA + idexReg.imm;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = false;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = true;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 2;
	
	return true;
}

bool CPU::op_swl()
{
	//Checked 26/08/2021

	exmemReg.rd = 0;
	exmemReg.aluRes = idexReg.regA + idexReg.imm - 0x03;	//treated as sw but startin at the LSB
	exmemReg.memToReg = false;
	exmemReg.aluToReg = false;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = true;
	exmemReg.bytes = 4;

	return true;
}

bool CPU::op_sw()
{
	//Checked 05/07/2021

	exmemReg.rd = 0;
 	exmemReg.aluRes = idexReg.regA + idexReg.imm;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = false;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = true;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 4;

	return true;
}

bool CPU::op_swr()
{
	//Checked 26/08/2021

	exmemReg.rd = 0;
	exmemReg.aluRes = idexReg.regA + idexReg.imm;	//treated as sw
	exmemReg.memToReg = false;
	exmemReg.aluToReg = false;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = true;
	exmemReg.bytes = 4;

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
	//Checked 05/07/2021

	exmemReg.rd = idexReg.rd;
	exmemReg.aluRes = idexReg.regB << idexReg.shamt;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = true;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_srl()
{
	//Checked 05/07/2021

	exmemReg.rd = idexReg.rd;
	exmemReg.aluRes = idexReg.regB >> idexReg.shamt;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = true;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_sra()
{
	//Checked 05/07/2021

	exmemReg.rd = idexReg.rd;
	exmemReg.aluRes = (int32_t)idexReg.regB >> idexReg.shamt;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = true;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_sllv()
{
	//Checked 17/07/2021

	exmemReg.rd = idexReg.rd;
	exmemReg.aluRes = idexReg.regB << (idexReg.regA & 0x0000001f);
	exmemReg.memToReg = false;
	exmemReg.aluToReg = true;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_srlv()
{
	//Checked 17/07/2021

	exmemReg.rd = idexReg.rd;
	exmemReg.aluRes = idexReg.regB >> (idexReg.regA & 0x0000001f);
	exmemReg.memToReg = false;
	exmemReg.aluToReg = true;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_srav()
{
	//Checked 17/07/2021

	exmemReg.rd = idexReg.rd;
	exmemReg.aluRes = (int32_t)idexReg.regB >> (idexReg.regA & 0x0000001f);
	exmemReg.memToReg = false;
	exmemReg.aluToReg = true;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_jr()
{
	//Checked 05/07/2021
	 
	//Foreward PC to fetch stage
	pc = idexReg.regA;
	ifidReg.branchDelaySlot = true;

	exmemReg.rd = 0;
	exmemReg.aluRes = 0;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = false;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_jalr()
{
	//Checked 05/07/2021
	 
	//Foreward PC to fetch stage
	pc = idexReg.regA;
	ifidReg.branchDelaySlot = true;

	exmemReg.rd = idexReg.rd;
	exmemReg.aluRes = idexReg.pc + 8; //return to the istruction after the delay slot
	exmemReg.memToReg = false;
	exmemReg.aluToReg = true;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_syscall()
{
	//Checked 16/07/2021
	
	exception(static_cast<uint32_t>(exceptionCause::syscall));

	return true;
}

bool CPU::op_break()
{
	//Checked 16/07/2021
	 
	exception(static_cast<uint32_t>(exceptionCause::breakpoint));

	return true;
}

bool CPU::op_mfhi()
{
	//Checked 16/07/2021
	//TODO: Not Clock Correct - hi could be unavailable

	exmemReg.rd = idexReg.rd;
	exmemReg.aluRes = hi;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = true;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_mthi()
{
	//Checked 16/07/2021
	//TODO: Check if clock correct

	hi = idexReg.regA;

	exmemReg.rd = 0;
	exmemReg.aluRes = 0;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = false;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_mflo()
{
	//Checked 16/07/2021
	//TODO: Not Clock Correct - lo could be unavailable

	exmemReg.rd = idexReg.rd;
	exmemReg.aluRes = lo;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = true;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_mtlo()
{
	//Checked 16/07/2021
	//TODO: Not Clock Correct

	lo = idexReg.regA; 

	exmemReg.rd = 0;
	exmemReg.aluRes = 0;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = false;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_mult()
{
	//Checked 16/07/2021
	//TODO: Not Clock Correct

	int64_t a, b, r;

	a = (int64_t)(int32_t)idexReg.regA;
	b = (int64_t)(int32_t)idexReg.regB;
	r = a * b;
	hi = (uint32_t)(r >> 32);
	lo = (uint32_t)r;

	exmemReg.rd = 0;
	exmemReg.aluRes = 0;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = false;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_multu()
{
	//Checked 16/07/2021
	//TODO: Not Clock Correct

	uint64_t a, b, r;

	a = (uint64_t)idexReg.regA;
	b = (uint64_t)idexReg.regB;
	r = a * b;
	hi = (uint32_t)(r >> 32);
	lo = (uint32_t)r;

	exmemReg.rd = 0;
	exmemReg.aluRes = 0;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = false;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_div()
{
	//Checked 16/07/2021
	//TODO: Not Clock Correct

	int32_t n, d;

	n = idexReg.regA;
	d = idexReg.regB;

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

	exmemReg.rd = 0;
	exmemReg.aluRes = 0;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = false;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_divu()
{
	//Checked 16/07/2021
	//TODO: Not Clock Correct

	int32_t n, d;

	n = idexReg.regA;
	d = idexReg.regB;

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

	exmemReg.rd = 0;
	exmemReg.aluRes = 0;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = false;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_add()
{
	//Checked 05/07/2021
	 
	//Check for integer overflow
	bool overflow = isOverflow((int32_t)idexReg.regA, (int32_t)idexReg.regB);

	exmemReg.rd = idexReg.rd;
	exmemReg.aluRes = (int32_t)idexReg.regA + (int32_t)idexReg.regB;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = !overflow; //Modify rd only if not overflow
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	if (overflow)
		exception(static_cast<uint32_t>(exceptionCause::overflow));

	return true;
}

bool CPU::op_addu()
{
	//Checked 05/07/2021

	exmemReg.rd = idexReg.rd;
	exmemReg.aluRes = (uint32_t)idexReg.regA + (uint32_t)idexReg.regB;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = true;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_sub()
{
	//Checked 16/07/2021

	//Check for integer overflow
	bool overflow = isOverflow((int32_t)idexReg.regA, (int32_t)idexReg.regB);

	exmemReg.rd = idexReg.rd;
	exmemReg.aluRes = idexReg.regA - idexReg.regB;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = true;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	if (overflow)
		exception(static_cast<uint32_t>(exceptionCause::overflow));
	
	return true;
}

bool CPU::op_subu()
{
	//Checked 05/07/2021

	exmemReg.rd = idexReg.rd;
	exmemReg.aluRes = (uint32_t)idexReg.regA - (uint32_t)idexReg.regB;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = true;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_and()
{
	//Checked 05/07/2021

	exmemReg.rd = idexReg.rd;
	exmemReg.aluRes = idexReg.regA & idexReg.regB;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = true;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_or()
{
	//Checked 05/07/2021

	exmemReg.rd = idexReg.rd;
	exmemReg.aluRes = idexReg.regA | idexReg.regB;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = true;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_xor()
{
	//Checked 16/07/2021

	exmemReg.rd = idexReg.rd;
	exmemReg.aluRes = idexReg.regA ^ idexReg.regB;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = true;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_nor()
{
	//Checked 17/07/2021

	exmemReg.rd = idexReg.rd;
	exmemReg.aluRes = ~(idexReg.regA | idexReg.regB);
	exmemReg.memToReg = false;
	exmemReg.aluToReg = true;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_slt()
{
	//Checked 05/07/2021

	exmemReg.rd = idexReg.rd;
	exmemReg.aluRes = ((int32_t)idexReg.regA < (int32_t)idexReg.regB) ? 0x00000001 : 0x00000000;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = true;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_sltu()
{
	//Checked 05/07/2021

	exmemReg.rd = idexReg.rd;
	exmemReg.aluRes = ((uint32_t)idexReg.regA < (uint32_t)idexReg.regB) ? 0x00000001 : 0x00000000;
	exmemReg.memToReg = false;
	exmemReg.aluToReg = true;
	exmemReg.regB = idexReg.regB;
	exmemReg.readMem = false;
	exmemReg.readMemUnaligned = false;
	exmemReg.readMemSignExt = false;
	exmemReg.writeMem = false;
	exmemReg.writeMemUnaligned = false;
	exmemReg.bytes = 0;

	return true;
}

bool CPU::op_unknown()
{
	//Checked 16/07/2021

	exception(static_cast<uint32_t>(exceptionCause::resinst));
	
	return true;
}

