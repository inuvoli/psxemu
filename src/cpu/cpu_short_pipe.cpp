#include <loguru.hpp>
#include "cpu_short_pipe.h"
#include "psx.h"
#include "functions_name.h"

CPU::CPU()
{
	//Init CPU Registers
	pc = 0xbfc00000;
	hi = 0x00000000;
	lo = 0x00000000;
	std::memset(gpr, 0x00, sizeof(uint32_t) * 32);
	cacheReg = 0x00000000;

	//Init Coprocessors cop0 & cop2
	cop0 = std::make_shared<Cop0>(this);
	cop2 = std::make_shared<Cop2>(this);

	//Init Pipeline Registers and Status
	std::memset(&currentOpcode, 0x00, sizeof(decodedOpcode));
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
	std::memset(gpr, 0x00, sizeof(uint32_t) * 32);
	cacheReg = 0x00000000;

	//Reset Coprocessor Cop0 and Cop2
	cop0->reset();
	cop2->reset();

	//Init Pipeline Registers and Status
	std::memset(&currentOpcode, 0x00, sizeof(decodedOpcode));
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
	//Exception not supported by PSX Bios
	//Check if PC in unaligned
	//if ((bool)(vAddr % bytes))
	//	exception(static_cast<uint32_t>(cpu::exceptionCause::addrerrload));

	//TODO: Instruction Cache Management

	return psx->rdMem(vAddr, bytes);
}

inline uint32_t CPU::rdMem(uint32_t vAddr, uint8_t bytes)
{
	cop0::StatusRegister statusReg;

	statusReg.word = cop0->reg[12];

	//Exception not supported by PSX Bios
	//Check if vAddr in unaligned
	//if ((bool)(vAddr % bytes))
	//	exception(static_cast<uint32_t>(cpu::exceptionCause::addrerrload));

	//Check if Cache is Isolated
	if (statusReg.isc)
	{
		//TODO
	}

	//Check if Reading from Data Cache, aka ScratchPad (0x1f800000 - 0x1f8003ff)
	if (memRangeScratchpad.contains(vAddr)) return rdDataCache(vAddr, bytes);

	//Check if Reading from Cache Control Register (0xfffe0130)
	if (memRangeCacheRegs.contains(vAddr)) return cacheReg;
	
	return psx->rdMem(vAddr, bytes);
}

inline bool CPU::wrMem(uint32_t vAddr, uint32_t& data, uint8_t bytes, bool checkalign)
{
	cop0::StatusRegister statusReg;

	statusReg.word = cop0->reg[12];

	//Check if vAddr in unaligned
	if ((bool)(vAddr % bytes) && checkalign)
		exception(static_cast<uint32_t>(cpu::exceptionCause::addrerrstore));

	//Check if Cache is Isolated
	if (statusReg.isc)
	{
		//TODO:
		LOG_F(3 , "CPU - Write Isolated cache! (Unsupported) (addr: %08x bytes: %d)",vAddr, bytes);
		//return wrDataCache(vAddr, data, bytes);
		return true;
	}

	//Check if Writing to Data Cache, aka ScratchPad
	if (memRangeScratchpad.contains(vAddr)) return wrDataCache(vAddr, data, bytes);

	//Check if Writing to Cache Control Register (0xfffe0130)
	if (memRangeCacheRegs.contains(vAddr))
	{
		cacheReg = data;
		iCacheEnabled = (bool)(cacheReg & ICACHE_EN_MASK);
		dCacheEnabled = (bool)(cacheReg & DCACHE_EN1_MASK) && (bool)(cacheReg & DCACHE_EN2_MASK);
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
bool CPU::execute()
{
	bool bResult = true;

	//Stop CPU if DMA is Running
	if (dmaTakeOnBus)
		return true;

	if (pc == 0xbfc06ff0 && psx->mem->exeInfo.isPresent)
	{
		LOG_F(INFO, "Jumping to PSX EXE");
		pc = psx->mem->exeInfo.regPCValue;
		gpr[28] = psx->mem->exeInfo.reg28Value;
		gpr[29] = psx->mem->exeInfo.reg29Value;
		gpr[30] = psx->mem->exeInfo.reg30Value;
		
	}

	//Fetch Instruction from current PC and increment it
	cpu::Instruction opcode;
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
		currentOpcode.rd = (uint8_t)opcode.rd;
		currentOpcode.rt = opcode.rt;
		currentOpcode.rs = opcode.rs;
		currentOpcode.regA = gpr[currentOpcode.rs];
		currentOpcode.regB = gpr[currentOpcode.rt];
		currentOpcode.tgt = opcode.tgt;
		currentOpcode.shamt = (uint8_t)opcode.shamt;
		currentOpcode.cofun = opcode.cofun;
		currentOpcode.imm = (uint32_t)(int16_t)opcode.imm; //Sign Extended

		//Execute Current Instruction
		if (currentOpcode.op == 0x00)
		{
			//SPECIAL opcode
			bResult = (this->*functSet[currentOpcode.funct].operate)();
			if (!bResult)
				LOG_F(ERROR, "CPU - Unimplemented Function %s! [%02x]", functSet[currentOpcode.funct].mnemonic.c_str(), currentOpcode.funct);
		}
		else
		{
			bResult = (this->*instrSet[currentOpcode.op].operate)();
			if (!bResult)
				LOG_F(ERROR, "CPU - Unimplemented Instruction %s!", instrSet[currentOpcode.op].mnemonic.c_str());
		}
	}
				
	return bResult;	
}

bool CPU::exception(uint32_t cause)
{
	cop0::StatusRegister	statusReg;
	cop0::CauseRegister		causeReg;
	
	LOG_F(2, "CPU - Received Exception [PC: 0x%08x, Cause: %d, EPC: 0x%08x, CauseRegister: 0x%08x, StatusRegister: 0x%08x]", pc, cause, cop0->reg[14], cop0->reg[13], cop0->reg[12]);
	
	//Get Current values of Cause Register and Status Register
	statusReg.word = cop0->reg[12];
	causeReg.word = cop0->reg[13];

	//Disable Interrupt (shift 2 position left bit [0, 5] for SR)
	statusReg.stk = (statusReg.stk << 2) & 0x3f;
	
	//Check if exception occurred in a branch delay slot
	if (branchDelaySlot)
	{
		cop0->reg[14] = pc - 4;		//Set EPC to Jump Instruction if exception occurs in a Branch Delay Slot
		causeReg.bd = true;			//Set BD bit in cop0 CAUSE Register if exception occurs in a Branch Delay Slot
		branchDelaySlot = false;
	}
	else
	{
		cop0->reg[14] = pc;			//Set EPC to Current Instruction if we are not in a Branch Delay Slot
		causeReg.bd = false;		//Set BD bit in cop0 CAUSE Register if exception occurs in a Branch Delay Slot
	}
	
	//Set Exception Code in CAUSE Register
	causeReg.excode = cause;
		
	//Jump to exception handler
	if (statusReg.bev)
		pc = 0xbfc00180;
	else
		pc = 0x80000080;

	//Update Cause Register and Status Register
	cop0->reg[13] = causeReg.word;
	cop0->reg[12] = statusReg.word;

	LOG_F(2, "CPU - Throw Exception [PC: 0x%08x, Cause: %d, EPC: 0x%08x, CauseRegister: 0x%08x, StatusRegister: 0x%08x]", pc, cause, cop0->reg[14], cop0->reg[13], cop0->reg[12]);

	return true;
}

bool CPU::interrupt(uint8_t status)
{
	cop0::StatusRegister	statusReg;
	cop0::CauseRegister		causeReg;

	//Get Current values of Cause Register and Status Register
	statusReg.word = cop0->reg[12];
	causeReg.word = cop0->reg[13];

	//Set cop0r13.bit10 (Cause Register) according to INTn pin value, PSX only use Hw INT0
	causeReg.iphw = status;	
	cop0->reg[13] = causeReg.word;		//Update Cause Register

	//Check COP0 for Pending non masked Interrupts with iEc enabled.
	if ((bool)(statusReg.imhw & causeReg.iphw) & (bool)statusReg.iec)
	{
		LOG_F(2, "CPU - Hardware Interrupt 0 Triggered");
		exception(static_cast<uint32_t>(cpu::exceptionCause::interrupt));		
	}
		
	return true;
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
		LOG_F(ERROR, "CPU - Unknown Branch Instruction\n");
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

	//Debug -- Call Stack
	callstackinfo tmp;
	tmp.jumpaddr = branchAddress;
	tmp.pc = pc - 4;
	tmp.sp = gpr[29]; //stack pointer
	tmp.ra = 0x0; //return address
	tmp.func = "";
	callStack.write(tmp);

	return true;
}

bool CPU::op_jal()
{
	//Set branchAddress
	//New PC is evaluated starting from PC of the instruction in the delay slot
	branchAddress = (pc & 0xf0000000) + (currentOpcode.tgt << 2);
	branchDelaySlot = true;
	gpr[31] = pc + 4;	//Return to the istruction after the delay slot

	//Debug -- Call Stack
	callstackinfo tmp;
	tmp.jumpaddr = branchAddress;
	tmp.pc = pc - 4;
	tmp.sp = gpr[29]; //stack pointer
	tmp.ra = gpr[31]; //return address

	std::stringstream ss;
	ss << "function_" << std::hex << tmp.jumpaddr;
	tmp.func = ss.str();

	callStack.write(tmp);

 
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
		exception(static_cast<uint32_t>(cpu::exceptionCause::overflow));
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
	//Exception not supported by PSX Bios
	//if (!cop0->execute(currentOpcode.cofun))
	//	exception(static_cast<uint32_t>(cpu::exceptionCause::copunusable));
	cop0->execute(currentOpcode.cofun);
	return true;
}

bool CPU::op_cop1()
{
	//Exception not supported by PSX Bios
	//exception(static_cast<uint32_t>(cpu::exceptionCause::copunusable));

	return true;
}

bool CPU::op_cop2()
{
	//Exception not supported by PSX Bios
	//if (!cop2->execute(currentOpcode.cofun))
	//	exception(static_cast<uint32_t>(cpu::exceptionCause::copunusable));
	cop2->execute(currentOpcode.cofun);
	return true;
}

bool CPU::op_cop3()
{
	//Exception not supported by PSX Bios
	//exception(static_cast<uint32_t>(cpu::exceptionCause::copunusable));

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
	//Unaligned reading from memory
	//LWL point to the MSB in memory and overwrite offset+1 byte on rt starting from the left
	//leaving the other bytes untouched

	// rt  - destination register
	// rs  - contains base address
	// imm - offset

	if (currentOpcode.rt == 0)
		return true;
	
	uint32_t vAddr;
	uint32_t phAddr;
	uint32_t offset;
	uint32_t shift;
	uint32_t value, tmp;

	value = currentOpcode.regB;   						//copy current content of rt
	vAddr = currentOpcode.regA + currentOpcode.imm;		//sum base address and offset to get Virtual Address
	phAddr = vAddr & 0xfffffffc;						//get word address aligned in memory
	offset = vAddr & 0x00000003;						//get offset within the word aligned in memory
	shift = 8*(3-offset);

	tmp = rdMem(phAddr);
	tmp = tmp << shift;
	value = (value & lwlMask[offset]) | tmp;

	gpr[currentOpcode.rt] = value;

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
	//Unaligned reading from memory
	//LWR point to the LSB in memory and overwrite offset-4 byte on rt starting from the right
	//leaving the other bytes untouched

	// rt  - destination register
	// rs  - contains base address
	// imm - offset

	if (currentOpcode.rt == 0)
		return true;
	
	uint32_t vAddr;
	uint32_t phAddr;
	uint32_t offset;
	uint32_t shift;
	uint32_t value, tmp;

	value = currentOpcode.regB;   						//copy current content of rt
	vAddr = currentOpcode.regA + currentOpcode.imm;		//sum base address and offset to get Virtual Address
	phAddr = vAddr & 0xfffffffc;						//get word address aligned in memory
	offset = vAddr & 0x00000003;						//get offset within the word aligned in memory
	shift = 8*offset;

	tmp = rdMem(phAddr);
	tmp = tmp >> shift;
	value = (value & lwrMask[offset]) | tmp;

	gpr[currentOpcode.rt] = value;

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
	// To Check

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
	//To Check

	//Unaligned writing to memory is approximated with a 4 byte writing from LSB
	//for both SWL and SWR. Both act like SW but starting from a potentially
	//unaligned address.
	wrMem(currentOpcode.regA + currentOpcode.imm, currentOpcode.regB, 4, false);
		
	return true;
}

bool CPU::op_lwc0()
{
	//Checked 16/07/2021
	//Exception not supported by PSX Bios
	//exception(static_cast<uint32_t>(cpu::exceptionCause::copunusable));

	return true;
}

bool CPU::op_lwc1()
{
	//Checked 16/07/2021
	//Exception not supported by PSX Bios
	//exception(static_cast<uint32_t>(cpu::exceptionCause::copunusable));

	return true;
}

bool CPU::op_lwc2()
{
	//Checked 31/03/2023

	cop2->reg.data[currentOpcode.rt] = rdMem(currentOpcode.regA + currentOpcode.imm, 4);

	return true;
}

bool CPU::op_lwc3()
{
	//Checked 16/07/2021
	//Exception not supported by PSX Bios
	//exception(static_cast<uint32_t>(cpu::exceptionCause::copunusable));

	return true;
}

bool CPU::op_swc0()
{
	//Checked 16/07/2021
	//Exception not supported by PSX Bios
	//exception(static_cast<uint32_t>(cpu::exceptionCause::copunusable));

	return true;
}

bool CPU::op_swc1()
{
	//Checked 16/07/2021
	//Exception not supported by PSX Bios
	//exception(static_cast<uint32_t>(cpu::exceptionCause::copunusable));

	return true;
}

bool CPU::op_swc2()
{
	//Checked 31/03/2023

	wrMem(currentOpcode.regA + currentOpcode.imm, cop2->reg.data[currentOpcode.rt], 4);
		
	return true;
}

bool CPU::op_swc3()
{
	//Checked 16/07/2021
	//Exception not supported by PSX Bios
	//exception(static_cast<uint32_t>(cpu::exceptionCause::copunusable));

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

	//Debug -- Call Stack
	callstackinfo tmp;
	tmp.jumpaddr = branchAddress;
	tmp.pc = pc - 4;
	tmp.sp = gpr[29]; //stack pointer
	tmp.ra = 0x0; //return address
	tmp.func = "";

	//DEBUG - stdlib calls
	//A Functions
	if (pc == 0xac)
	{
		LOG_F(1, "CPU - %s (%08x, %08x, %08x, %08x) [A(%02xh)]", function_A[gpr[9]].c_str(), gpr[4], gpr[5], gpr[6],gpr[7], gpr[9]);
		tmp.func = function_A[gpr[9]];
	}
	//B Functions
	if (pc == 0xbc && gpr[9] != 0x3d) //exclude std_out_putchar B(3dh)
	{
		LOG_F(1, "CPU - %s (%08x, %08x, %08x, %08x) [B(%02xh)]", function_B[gpr[9]].c_str(), gpr[4], gpr[5], gpr[6],gpr[7], gpr[9]);
		tmp.func = function_B[gpr[9]];
	}
	//C Functions
	if (pc == 0xcc)
	{
		LOG_F(1, "CPU - %s (%08x, %08x, %08x, %08x) [C(%02xh)]", function_C[gpr[9]].c_str(), gpr[4], gpr[5], gpr[6],gpr[7], gpr[9]);
		tmp.func = function_C[gpr[9]];
	}
	
	callStack.write(tmp);

	return true;
}

bool CPU::op_jalr()
{
	//Forward PC to fetch stage
	branchAddress = currentOpcode.regA;
	branchDelaySlot = true;
	gpr[31] = pc + 4;

	//Debug -- Call Stack
	callstackinfo tmp;
	tmp.jumpaddr = branchAddress;
	tmp.pc = pc - 4;
	tmp.sp = gpr[29]; //stack pointer
	tmp.ra = gpr[31]; //return address

	std::stringstream ss;
	ss << "function_" << std::hex << tmp.jumpaddr;
	tmp.func = ss.str();
	
	callStack.write(tmp);

	return true;
}

bool CPU::op_syscall()
{
	LOG_F(1, "CPU - Calling %s [SYS(%02xh)]", function_SYS[gpr[4] & 0x0000000f].c_str(), gpr[4]);
	exception(static_cast<uint32_t>(cpu::exceptionCause::syscall));

	return true;
}

bool CPU::op_break()
{
	//Exception not supported by PSX Bios
	//exception(static_cast<uint32_t>(cpu::exceptionCause::breakpoint));

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
		exception(static_cast<uint32_t>(cpu::exceptionCause::overflow));

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
		exception(static_cast<uint32_t>(cpu::exceptionCause::overflow));
	
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
	//Exception not supported by PSX Bios
	//exception(static_cast<uint32_t>(cpu::exceptionCause::resinst));
	return false;
}

