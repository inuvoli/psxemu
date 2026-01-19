#include <loguru.hpp>
#include "cpu_short_pipe.h"
#include "psx.h"

CpuShort::CpuShort()
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
	isInDelaySlot = false;
	branchAddress = 0x00000000;
	branchFunctionAddress = 0x00000000;

	//Init Call Stack Callback
	kernelCallCb = nullptr;

	//Init Memory Delay Load Status
	currentDelayedRegisterLoad.id = 0;
	currentDelayedRegisterLoad.value = 0;
	nextDelayedRegisterLoad.id = 0;
	nextDelayedRegisterLoad.value = 0;

	//Init Instruction & Function Dictionaries
	instrSet =
	{
		{"special", &CpuShort::op_unknown},
		{"bxx rs, imm", &CpuShort::op_bxx},
		{"j tgt", &CpuShort::op_j},
		{"jal tgt", &CpuShort::op_jal},
		{"beq rs, rt, imm", &CpuShort::op_beq},
		{"bne rs, rt, imm", &CpuShort::op_bne},
		{"blez rs, imm", &CpuShort::op_blez},
		{"bgtz rs, imm", &CpuShort::op_bgtz},
		{"addi rt, rs, imm", &CpuShort::op_addi},
		{"addiu rt, rs, imm", &CpuShort::op_addiu},
		{"slti rt, rs, imm", &CpuShort::op_slti},
		{"sltiu rt, rs, imm", &CpuShort::op_sltiu},
		{"andi rt, rs, imm", &CpuShort::op_andi},
		{"ori rt, rs, imm", &CpuShort::op_ori},
		{"xori rt, rs, imm", &CpuShort::op_xori},
		{"lui rt, imm", &CpuShort::op_lui},
		{"cop0 cofun", &CpuShort::op_cop0},
		{"cop1 cofun", &CpuShort::op_cop1},
		{"cop2 cofun", &CpuShort::op_cop2},
		{"cop3 cofun", &CpuShort::op_cop3},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"lb rt, imm(rs)", &CpuShort::op_lb},
		{"lh rt, imm(rs)", &CpuShort::op_lh},
		{"lwl rt, imm(rs)", &CpuShort::op_lwl},
		{"lw rt, imm(rs)", &CpuShort::op_lw},
		{"lbu rt, imm(rs)", &CpuShort::op_lbu},
		{"lhu rt, imm(rs)", &CpuShort::op_lhu},
		{"lwr rt, imm(rs)", &CpuShort::op_lwr},
		{"invalid operation", &CpuShort::op_unknown},
		{"sb rt, imm(rs)", &CpuShort::op_sb},
		{"sh rt, imm(rs)", &CpuShort::op_sh},
		{"swl rt, imm(rs)", &CpuShort::op_swl},
		{"sw rt, imm(rs)", &CpuShort::op_sw},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"swr rt, imm(rs)", &CpuShort::op_swr},
		{"invalid operation", &CpuShort::op_unknown},
		{"lwc0 rt, imm(rs)", &CpuShort::op_lwc0},
		{"lwc1 rt, imm(rs)", &CpuShort::op_lwc1},
		{"lwc2 rt, imm(rs)", &CpuShort::op_lwc2},
		{"lwc3 rt, imm(rs)", &CpuShort::op_lwc3},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"swc0 rt, imm(rs)", &CpuShort::op_swc0},
		{"swc1 rt, imm(rs)", &CpuShort::op_swc1},
		{"swc2 rt, imm(rs)", &CpuShort::op_swc2},
		{"swc3 rt, imm(rs)", &CpuShort::op_swc3},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown}
	};

	functSet =
	{
		{"sll rd, rt, shamt", &CpuShort::op_sll},
		{"invalid operation", &CpuShort::op_unknown},
		{"srl rd, rt, shamt", &CpuShort::op_srl},
		{"sra rd,rt, shamt", &CpuShort::op_sra},
		{"sllv rd, rt, rs", &CpuShort::op_sllv},
		{"invalid operation", &CpuShort::op_unknown},
		{"srlv rd, rt, rs", &CpuShort::op_srlv},
		{"srav rd, rt, rs", &CpuShort::op_srav},
		{"jr rs", &CpuShort::op_jr},
		{"jalr rd, rs", &CpuShort::op_jalr},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"syscall", &CpuShort::op_syscall},
		{"break", &CpuShort::op_break},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"mfhi rd", &CpuShort::op_mfhi},
		{"mthi rs", &CpuShort::op_mthi},
		{"mflo rd", &CpuShort::op_mflo},
		{"mtlo rs", &CpuShort::op_mtlo},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"mult rs, rt", &CpuShort::op_mult},
		{"multu rs, rt", &CpuShort::op_multu},
		{"div rs, rt", &CpuShort::op_div},
		{"divu rs, rt", &CpuShort::op_divu},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"add rd, rs, rt", &CpuShort::op_add},
		{"addu rd, rs, rt", &CpuShort::op_addu},
		{"sub rd, rs, rt", &CpuShort::op_sub},
		{"subu rd, rs, rt", &CpuShort::op_subu},
		{"and rd, rs, rt", &CpuShort::op_and},
		{"or rd, rs, rt", &CpuShort::op_or},
		{"xor rd, rs, rt", &CpuShort::op_xor},
		{"nor rd, rs, rt", &CpuShort::op_nor},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"slt rd, rs, rt", &CpuShort::op_slt},
		{"sltu rd, rs, rt", &CpuShort::op_sltu},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown},
		{"invalid operation", &CpuShort::op_unknown}
	};
}

CpuShort::~CpuShort()
{
}

void CpuShort::link(Psx *instance)
{
	psx = instance;
}

bool CpuShort::reset()
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
	isInDelaySlot = false;
	branchAddress = 0x00000000;
	branchFunctionAddress = 0x00000000;

	//Init Memory Delay Load Status
	currentDelayedRegisterLoad.id = 0;
	currentDelayedRegisterLoad.value = 0;
	nextDelayedRegisterLoad.id = 0;
	nextDelayedRegisterLoad.value = 0;


	
	return true;
}

//-----------------------------------------------------------------------------------------------------------------------------------
//
// Cache and Memory Access Functions
//
//-----------------------------------------------------------------------------------------------------------------------------------
inline uint32_t CpuShort::rdInst(uint32_t vAddr, uint8_t bytes)
{
	//TODO: Instruction Cache Management

	return psx->rdMem(vAddr, bytes);
}

inline uint32_t CpuShort::rdMem(uint32_t vAddr, uint8_t bytes)
{
	cop0::StatusRegister statusReg;

	statusReg.word = cop0->reg[12];

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

inline bool CpuShort::wrMem(uint32_t vAddr, uint32_t& data, uint8_t bytes, bool checkalign)
{
	cop0::StatusRegister statusReg;

	statusReg.word = cop0->reg[12];

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

uint32_t CpuShort::rdDataCache(uint32_t vAddr, uint8_t bytes)
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

bool CpuShort::wrDataCache(uint32_t vAddr, uint32_t& data, uint8_t bytes)
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

uint32_t CpuShort::rdInstrCache(uint32_t vAddr, uint8_t bytes)
{
	return uint32_t();
}

bool CpuShort::wrInstrCache(uint32_t vAddr, uint32_t& data, uint8_t bytes)
{
	return false;
}

//-------------------------------------------------------------------------------------------------------------
//Delay Memory Load Helper fFunctions
//-------------------------------------------------------------------------------------------------------------

// Write a general-purpose register immediately.
// Parameters:
// - id:	index of the GPR to write (0-31). Writes to register 0 should be ignored.
// - value: value to store into the register.
// Returns true on success, false on failure.
bool CpuShort::writeRegister(uint8_t id, uint32_t value)
{	
	if (id == 0)
		return true;

	//Write value on the register
	gpr[id] = value;

	//Check if we are writing the same register on the Current Delay, in that case the Current Delay Load is discarded
	if (currentDelayedRegisterLoad.id == id)
	{
		currentDelayedRegisterLoad.id = 0;
		currentDelayedRegisterLoad.value = 0;
	}

	return true;
}

// Schedule a register write to occur after the next instruction (delayed load).
// This models the MIPS load-delay behavior where loads take effect one
// instruction later.
// Parameters:
// - id:	index of the GPR to write (0-31). Writes to register 0 should be ignored.
// - value: value to store into the register.
//Return true if the delayed write was scheduled.
bool CpuShort::writeRegisterDelayed(uint8_t id, uint32_t value)
{
	if (id == 0)
		return true;
	
	nextDelayedRegisterLoad.id = id;
	nextDelayedRegisterLoad.value = value;

	//Check if we are rewriting the same register on the Current Delay, in that case the Current Delay Load is discarded
	if (currentDelayedRegisterLoad.id == id)
	{
		currentDelayedRegisterLoad.id = 0;
		currentDelayedRegisterLoad.value = 0;
	}

	return true;
}

// Used by LWL and LWR, read rt in flight if is pending it's update for Memory Delay
// Parameters:
// - id:	index of the GPR to write (0-31). Writes to register 0 should be ignored.
// Return:
// In Flight register value if LWR or LWL uses the same register in the Current Delay Load for rt
uint32_t CpuShort::readRegisterInFlight(uint8_t id)
{
	
	// the content of the register otherwise

	if (id == currentDelayedRegisterLoad.id)
		return currentDelayedRegisterLoad.value;
	else
		return gpr[id];
}

// Commit any pending delayed register loads to the architectural register
// file. Returns true if a delayed load was applied, false if there was
// nothing to do.
bool CpuShort::performDelayedLoad()
{
	gpr[currentDelayedRegisterLoad.id] = currentDelayedRegisterLoad.value;

	currentDelayedRegisterLoad.id = nextDelayedRegisterLoad.id;
	currentDelayedRegisterLoad.value = nextDelayedRegisterLoad.value;
	nextDelayedRegisterLoad.id = 0;
	nextDelayedRegisterLoad.value = 0;

	return true;
}

//-----------------------------------------------------------------------------------------------------------------------------------
//
// Pipeline Implementation
//
//-----------------------------------------------------------------------------------------------------------------------------------
bool CpuShort::execute()
{
	cpu::Instruction opcode;

	auto runInstruction = [&]()
	{
		bool bResult;

		//Decode Current Instruction Fields
		currentOpcode.op = opcode.op;
		currentOpcode.funct = opcode.funct;
		currentOpcode.rd = (uint8_t)opcode.rd;
		currentOpcode.rt = (uint8_t)opcode.rt;
		currentOpcode.rs = (uint8_t)opcode.rs;
		currentOpcode.regA = gpr[currentOpcode.rs];
		currentOpcode.regB = gpr[currentOpcode.rt];
		currentOpcode.tgt = opcode.tgt;
		currentOpcode.shamt = (uint8_t)opcode.shamt;
		currentOpcode.cofun = (uint32_t)opcode.cofun;
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

		//Memory Delay Load implementation
		performDelayedLoad();

		return bResult;
	};
	
	//Stall CPU if databus is busy
	if (psx->dataBusBusy)
		return true;
		
	//Check for Shell Execution
	if (pc == 0x80030000)
	{
		LOG_F(INFO, "CPU - Shell Execution Detected, loading EXE if present...");
		std::string fileName = commandline::instance().getExeFileName();
		if (!fileName.empty())
		{
			psx->exeFile->loadExe(fileName, psx->mem->ram);
			psx->exeFile->setRegisters(&pc, gpr);
			LOG_F(INFO, "CPU - EXE Loaded, jumping to 0x%08x", pc);	
		}
		else
		{
			LOG_F(ERROR, "CPU - No EXE specified, continuing BIOS execution...");
		}
	}

	//Fetch Instruction from current PC
	opcode.word = rdInst(pc);

	//Check if isInDelaySlot is set, in that case we are executing the instruction in the Branch Delay Slot
	//and Program Counter must be set to branch new value, if not just step to the next instruction
	if (isInDelaySlot)
	{
		pc = branchAddress;
		runInstruction();
		isInDelaySlot = false;
		
		//Check for Kernel Call and trigger Callback
		if(kernelCallCb)
		{
			KernelCallEvent e;
			e.pc = pc;
			e.sp = gpr[29];
			e.ra = gpr[31];
			e.t1 = gpr[9];
			e.a0 = gpr[4];
			e.a1 = gpr[5];
			e.a2 = gpr[6];
			e.a3 = gpr[7];
			kernelCallCb(e);
		}
	}
	else
	{
		pc += 4;
		runInstruction();
	}

	return true;
}

bool CpuShort::exception(uint32_t cause)
{
	cop0::StatusRegister	statusReg;
	cop0::CauseRegister		causeReg;
	
	//Get Current values of Cause Register and Status Register
	statusReg.word = cop0->reg[12];
	causeReg.word = cop0->reg[13];

	
	//Disable Interrupt (shift 2 position left Status Register bit [0, 5])
	//Set Kernel Mode and Disable Interrupts
	statusReg.stk = (statusReg.stk << 2) & 0x3f;
	statusReg.iec = false;	//Disable Interrupts
	statusReg.kuc = true;	//Set to Kernel Mode

	//Set Exception Code in CAUSE Register
	causeReg.excode = static_cast<uint8_t>(cause & 0x1f);
		
	//Update EPC and BD bit on Status Register according to exception origin
	//In case of exception in a delay slot, EPC is set to the branch instruction address
	//and TAR is set to the branch destination address.
	//In case of exception not in a delay slot, EPC is set to the failing instruction address
	//Update TAR [Cop0r6] Cop0 Register 6 is always set to the address after the failing instruction
	if (isInDelaySlot)
	{
		cop0->reg[14] = branchFunctionAddress;
		cop0->reg[6] = branchAddress;
		causeReg.bd = true;
	}
	else
	{
		//An original R3000A CPU never sets EPC to the address of a memory delay load instruction (ie. LW)
		//EPC is always set to the next istruction address in this cases.
		if (currentDelayedRegisterLoad.id != 0)
			cop0->reg[14] = pc; //Next instruction address
		else
			cop0->reg[14] = pc - 4;	 //Failing instruction address
		causeReg.bd = false;
	}

	//PSX always jumps to 0x80000080 on exception, don't care about BEV bit
	pc = 0x80000080;
	//if (statusReg.bev)
	//	pc = 0xbfc00180;

	//Update Cause Register and Status Register
	cop0->reg[13] = causeReg.word;
	cop0->reg[12] = statusReg.word;

	LOG_F(2, "CPU - Throw Exception [PC: 0x%08x, Cause: %d, EPC: 0x%08x, CauseRegister: 0x%08x, StatusRegister: 0x%08x]", pc, cause, cop0->reg[14], cop0->reg[13], cop0->reg[12]);

	return true;
}

bool CpuShort::interrupt(uint8_t number, bool status)
{
	cop0::StatusRegister	statusReg;
	cop0::CauseRegister		causeReg;

	//Get Current values of Status Register and Cause Register
	statusReg.word = cop0->reg[12];
	causeReg.word = cop0->reg[13];

	//Update IP Field for Cause Register, it mirrors the value of i_stat & i_mask for HW Interrupt 0
	causeReg.iphw = 0;
	if (status)
		causeReg.iphw = 1;

	//If the hw interrupt is enabled and the global interrupt are enabled throw an exception
	//PSX only use HW Interrupt INT0
	if ((causeReg.iphw && statusReg.imhw) && statusReg.iec)
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
bool CpuShort::op_bxx()
{
	bool branch = false;

	switch (currentOpcode.rt)
	{
	case 0x00:
	case 0x02:
	case 0x04:
	case 0x06:
	case 0x08:
	case 0x0a:
	case 0x0c:
	case 0x0e:
	case 0x12:
	case 0x14:
	case 0x16:
	case 0x18:
	case 0x1a:
	case 0x1c:
	case 0x1e:
		//BLTZ
		branch = ((int32_t)currentOpcode.regA < 0);
		break;
	case 0x01:
	case 0x03:
	case 0x05:
	case 0x07:
	case 0x09:
	case 0x0b:
	case 0x0d:
	case 0x0f:
	case 0x13:
	case 0x15:
	case 0x17:
	case 0x19:
	case 0x1b:
	case 0x1d:
	case 0x1f:
		//BGEZ
		branch = ((int32_t)currentOpcode.regA >= 0);
		break;
	case 0x10:
		//BLTZAL
		branch = ((int32_t)currentOpcode.regA < 0);
		writeRegister(31, pc + 4); //Return to the istruction after the delay slot
		break;
	case 0x11:
		//BGEZAL
		branch = ((int32_t)currentOpcode.regA >= 0);
		writeRegister(31, pc + 4); //Return to the istruction after the delay slot
		break;
	default:
		branch = false;
		LOG_F(ERROR, "CPU - Unknown Branch Instruction [%d]\n", currentOpcode.rt);
		return false;
	}

	//Set branchAddress if Branch is triggered
	//New PC is evaluated starting from PC of the instruction in the delay slot
	if (branch)
	{
		branchAddress = pc + (currentOpcode.imm << 2);
		branchFunctionAddress = pc - 4;
		isInDelaySlot = true;
	}

	return true;
}

bool CpuShort::op_j()
{
	//Set branchAddress
	//New PC is evaluated starting from PC of the instruction in the delay slot
	branchAddress = (pc & 0xf0000000) + (currentOpcode.tgt << 2);
	branchFunctionAddress = pc - 4;
	isInDelaySlot = true;

	return true;
}

bool CpuShort::op_jal()
{
	//Set branchAddress
	//New PC is evaluated starting from PC of the instruction in the delay slot
	branchAddress = (pc & 0xf0000000) + (currentOpcode.tgt << 2);
	branchFunctionAddress = pc - 4;
	isInDelaySlot = true;

	//Set Return Address
	//Should return to the istruction after the delay slot instruction. PC already point at the delay slot at this stage
	writeRegister(31, pc + 4);
 
	return true;
}

bool CpuShort::op_beq()
{
	//Set branchAddress if Branch is triggered
	//New PC is evaluated starting from PC of the instruction in the delay slot
	if (currentOpcode.regA == currentOpcode.regB)
	{
		branchAddress = pc + (currentOpcode.imm << 2);
		branchFunctionAddress = pc - 4;
		isInDelaySlot = true;
	}

	return true;
}

bool CpuShort::op_bne()
{
	//Set branchAddress if Branch is triggered
	//New PC is evaluated starting from PC of the instruction in the delay slot
	if (currentOpcode.regA != currentOpcode.regB)
	{
		branchAddress = pc + (currentOpcode.imm << 2);
		branchFunctionAddress = pc - 4;
		isInDelaySlot = true;
	}

	return true;
}

bool CpuShort::op_blez()
{
	//Set branchAddress if Branch is triggered
	//New PC is evaluated starting from PC of the instruction in the delay slot
	if ((int32_t)currentOpcode.regA <= 0)
	{
		branchAddress = pc + (currentOpcode.imm << 2);
		branchFunctionAddress = pc - 4;
		isInDelaySlot = true;
	}

	return true;
}

bool CpuShort::op_bgtz()
{
	//Set branchAddress if Branch is triggered
	//New PC is evaluated starting from PC of the instruction in the delay slot
	if ((int32_t)currentOpcode.regA > 0)
	{
		branchAddress = pc + (currentOpcode.imm << 2);
		branchFunctionAddress = pc - 4;
		isInDelaySlot = true;
	}

	return true;
}

bool CpuShort::op_addi()
{
	uint32_t value = currentOpcode.regA;
	uint32_t imm = currentOpcode.imm;
    uint32_t result = value + imm;

    if (!((value ^ imm) & 0x80000000) && ((result ^ value) & 0x80000000))
	{
        exception(static_cast<uint32_t>(cpu::exceptionCause::overflow));
		return true;
    }
	else
	{
		if (currentOpcode.rt !=0)
			writeRegister(currentOpcode.rt, result);
    }

	return true;
}

bool CpuShort::op_addiu()
{
	uint32_t result = currentOpcode.regA + currentOpcode.imm;

	if (currentOpcode.rt != 0)
		writeRegister(currentOpcode.rt, result);

	return true;
}

bool CpuShort::op_slti()
{
	uint32_t result = ((int32_t)currentOpcode.regA < (int32_t)currentOpcode.imm) ? 0x00000001 : 0x00000000;

	if (currentOpcode.rt != 0)
		writeRegister(currentOpcode.rt, result);

	return true;
}

bool CpuShort::op_sltiu()
{
	uint32_t result = ((uint32_t)currentOpcode.regA < (uint32_t)currentOpcode.imm) ? 0x00000001 : 0x00000000;

	if (currentOpcode.rt != 0)
		writeRegister(currentOpcode.rt, result);
	
	return true;
}

bool CpuShort::op_andi()
{
	uint32_t result = currentOpcode.regA & (0x0000ffff & currentOpcode.imm); //imm is zero extended

	if (currentOpcode.rt != 0)
		writeRegister(currentOpcode.rt, result);

	return true;
}

bool CpuShort::op_ori()
{
	uint32_t result = currentOpcode.regA | (0x0000ffff & currentOpcode.imm); //imm is zero extended

	if (currentOpcode.rt != 0)
		writeRegister(currentOpcode.rt, result);

	return true;
}

bool CpuShort::op_xori()
{
	uint32_t result = currentOpcode.regA ^ (0x0000ffff & currentOpcode.imm); //imm is zero extended

	if (currentOpcode.rt != 0)
		writeRegister(currentOpcode.rt, result);
		
	return true;
}

bool CpuShort::op_lui()
{
	uint32_t result = currentOpcode.imm << 16;

	if (currentOpcode.rt != 0)
		writeRegister(currentOpcode.rt, result);

	return true;
}

bool CpuShort::op_cop0()
{
	//Exception not supported by PSX Bios
	//if (!cop0->execute(currentOpcode.cofun))
	//	exception(static_cast<uint32_t>(cpu::exceptionCause::copunusable));
	cop0->execute(currentOpcode.cofun);
	return true;
}

bool CpuShort::op_cop1()
{
	//Exception not supported by PSX Bios
	//exception(static_cast<uint32_t>(cpu::exceptionCause::copunusable));

	return true;
}

bool CpuShort::op_cop2()
{
	//Exception not supported by PSX Bios
	//if (!cop2->execute(currentOpcode.cofun))
	//	exception(static_cast<uint32_t>(cpu::exceptionCause::copunusable));
	cop2->execute(currentOpcode.cofun);
	return true;
}

bool CpuShort::op_cop3()
{
	//Exception not supported by PSX Bios
	//exception(static_cast<uint32_t>(cpu::exceptionCause::copunusable));

	return true;
}

bool CpuShort::op_lb()
{
	uint32_t imm = currentOpcode.imm;;
    uint32_t address = currentOpcode.regA + imm;
    
    uint32_t value = (int8_t)rdMem(address, 1);

	//Set Memory Load Delay info
	writeRegisterDelayed(currentOpcode.rt, value);

	return true;
}

bool CpuShort::op_lh()
{
	uint32_t imm = currentOpcode.imm;;
    uint32_t targetAddress = currentOpcode.regA + imm;
    
	//Check Target Address Alignment, last bit must be 0
	if (targetAddress & 0x1)
	{
		LOG_F(ERROR, "CPU - Address Load Error: 0x%08x lh r%d, 0x%08x(r%d)  - [0x%08x], [0x%08x]", pc - 4, currentOpcode.rt, currentOpcode.imm, currentOpcode.rs, gpr[currentOpcode.rd], currentOpcode.regA);

		//Exception not supported by PSX Bios
		cop0->reg[8] = targetAddress;	//Bad Virtual Address
        exception(static_cast<uint32_t>(cpu::exceptionCause::addrerrload));
		
        return true;
    }
    
	//Set Memory Load Delay info
	writeRegisterDelayed(currentOpcode.rt, (int16_t)rdMem(targetAddress, 2));
	
	return true;
}

bool CpuShort::op_lwl()
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

	value = readRegisterInFlight(currentOpcode.rt);   	//copy current content of rt
	vAddr = currentOpcode.regA + currentOpcode.imm;		//sum base address and offset to get Virtual Address
	phAddr = vAddr & 0xfffffffc;						//get word address aligned in memory
	offset = vAddr & 0x00000003;						//get offset within the word aligned in memory
	shift = 8*(3-offset);

	tmp = rdMem(phAddr);
	tmp = tmp << shift;
	value = (value & lwlMask[offset]) | tmp;

	//Set Memory Load Delay info
	writeRegisterDelayed(currentOpcode.rt, value);

	return true;
}

bool CpuShort::op_lw()
{
	uint32_t offset = currentOpcode.imm;
    uint32_t targetAddress = currentOpcode.regA + offset;

	//Check Target Address Alignment, last two bits must be 0
    if (targetAddress & 0x3)
	{
		LOG_F(ERROR, "CPU - Address Load Error: 0x%08x lw r%d, 0x%08x(r%d)  - [rt: 0x%08x], [rs: 0x%08x] [MasterClock: %d]", pc - 4, currentOpcode.rt, offset, currentOpcode.rs, gpr[currentOpcode.rt], gpr[currentOpcode.rs], psx->masterClock);
        
		//Exception not supported by PSX Bios
		cop0->reg[8] = targetAddress;	//Bad Virtual Address
        exception(static_cast<uint32_t>(cpu::exceptionCause::addrerrload));

		return true;
    }

	//Set Memory Load Delay info
	writeRegisterDelayed(currentOpcode.rt, (uint32_t)rdMem(targetAddress, 4));
	
	return true;
}

bool CpuShort::op_lbu()
{
	//Set Memory Load Delay info
	writeRegisterDelayed(currentOpcode.rt, rdMem(currentOpcode.regA + currentOpcode.imm, 1));
	
	return true;
}

bool CpuShort::op_lhu()
{
	uint32_t imm = currentOpcode.imm;;
    uint32_t targetAddress = currentOpcode.regA + imm;
    
	//Check Target Address Alignment, last bit must be 0
	if (targetAddress & 0x1)
	{
		LOG_F(ERROR, "CPU - Address Load Error: 0x%08x lhu r%d, 0x%08x(r%d)  - [0x%08x], [0x%08x]", pc - 4, currentOpcode.rt, currentOpcode.imm, currentOpcode.rs, gpr[currentOpcode.rd], currentOpcode.regA);
        
		//Exception not supported by PSX Bios
		cop0->reg[8] = targetAddress;	//Bad Virtual Address
        exception(static_cast<uint32_t>(cpu::exceptionCause::addrerrload));
		
		return true;
    }

	//Set Memory Load Delay info
	writeRegisterDelayed(currentOpcode.rt, (uint16_t)rdMem(targetAddress, 2));
	
	return true;
}

bool CpuShort::op_lwr()
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

	value = readRegisterInFlight(currentOpcode.rt);   	//copy current content of rt
	vAddr = currentOpcode.regA + currentOpcode.imm;		//sum base address from rs and offset to get Virtual Address
	phAddr = vAddr & 0xfffffffc;						//get word address aligned in memory
	offset = vAddr & 0x00000003;						//get offset within the word aligned in memory
	shift = 8*offset;

	tmp = rdMem(phAddr);
	tmp = tmp >> shift;
	value = (value & lwrMask[offset]) | tmp;

	//Set Memory Load Delay info
	writeRegisterDelayed(currentOpcode.rt, value);
	
	return true;
}

bool CpuShort::op_sb()
{
	//Checked 05/07/2021
	wrMem(currentOpcode.regA + currentOpcode.imm, currentOpcode.regB, 1);

	return true;
}

bool CpuShort::op_sh()
{
	uint32_t imm = currentOpcode.imm;
    uint32_t targetAddress = currentOpcode.regA + imm;

	//Check Target Address Alignment, last bit must be 0
    if (targetAddress & 0x1)
	{
        cop0->reg[8] = targetAddress;
        exception(static_cast<uint32_t>(cpu::exceptionCause::addrerrstore));
        return true;
    }

    uint32_t value = currentOpcode.regB;
	wrMem(targetAddress, value, 2);

	return true;
}

bool CpuShort::op_swl()
{

	uint32_t imm = currentOpcode.imm;    
    uint32_t address = currentOpcode.regA + imm;
    uint32_t value = currentOpcode.regB;
    uint32_t alignedAddress = address & 0xfffffffc;
	uint32_t currentMemoryValue = (uint32_t)rdMem(alignedAddress);

    uint32_t memoryValue;
    switch (address & 3)
	{
        case 0:
	
            memoryValue = (currentMemoryValue & 0xffffff00) | (value >> 24);
            break;
    
        case 1:
            memoryValue = (currentMemoryValue & 0xffff0000) | (value >> 16);
            break;
        
		case 2: 
            memoryValue = (currentMemoryValue & 0xff000000) | (value >> 8);
            break;

        case 3:
            memoryValue = (currentMemoryValue & 0x00000000) | (value >> 0);
            break;
    }

	wrMem(alignedAddress, memoryValue);

	return true;
}

bool CpuShort::op_sw()
{
	uint32_t imm = currentOpcode.imm;
    uint32_t targetAddress = currentOpcode.regA + imm;

	//Check Target Address Alignment, last two bits must be 0
    if (targetAddress & 0x3)
	{
        cop0->reg[8] = targetAddress;
        exception(static_cast<uint32_t>(cpu::exceptionCause::addrerrstore));
        return true;
    }

    uint32_t value = currentOpcode.regB;
	wrMem(targetAddress, value, 4);

	return true;
}

bool CpuShort::op_swr()
{
	uint32_t imm = currentOpcode.imm;
    uint32_t address = currentOpcode.regA + imm;
    uint32_t value = currentOpcode.regB;
    uint32_t alignedAddress = address & 0xfffffffc;
	uint32_t currentMemoryValue = (uint32_t)rdMem(alignedAddress);

    uint32_t memoryValue;
    switch (address & 3) {
        case 0: {
            memoryValue = (currentMemoryValue & 0x00000000) | (value << 0);
            break;
        }
        case 1: {
            memoryValue = (currentMemoryValue & 0x000000ff) | (value << 8);
            break;
        }
        case 2: {
            memoryValue = (currentMemoryValue & 0x0000ffff) | (value << 16);
            break;
        }
        case 3: {
            memoryValue = (currentMemoryValue & 0x00ffffff) | (value << 24);
            break;
        }
    }

    wrMem(alignedAddress, memoryValue);

	return true;
}

bool CpuShort::op_lwc0()
{
	//Checked 16/07/2021
	//Exception not supported by PSX Bios
	//exception(static_cast<uint32_t>(cpu::exceptionCause::copunusable));

	return true;
}

bool CpuShort::op_lwc1()
{
	//Checked 16/07/2021
	//Exception not supported by PSX Bios
	//Does it throw exceptions?
	//CauseRegister CE field must be set to 1 to trigger this exception?
	//exception(static_cast<uint32_t>(cpu::exceptionCause::copunusable));

	return true;
}

bool CpuShort::op_lwc2()
{
	//Checked 31/03/2023
	//Does it throw exceptions?
	//CauseRegister CE field must be set to 2 to trigger this exception?
	cop2->reg.data[currentOpcode.rt] = rdMem(currentOpcode.regA + currentOpcode.imm, 4);

	return true;
}

bool CpuShort::op_lwc3()
{
	//Checked 16/07/2021
	//Exception not supported by PSX Bios
	//Does it throw exceptions?
	//CauseRegister CE field must be set to 3 to trigger this exception?
	//exception(static_cast<uint32_t>(cpu::exceptionCause::copunusable));

	return true;
}

bool CpuShort::op_swc0()
{
	//Checked 16/07/2021
	//Exception not supported by PSX Bios
	//Does it throw exceptions?
	//CoauseRegister CE field must be set to 0 to trigger this exception?
	//exception(static_cast<uint32_t>(cpu::exceptionCause::copunusable));

	return true;
}

bool CpuShort::op_swc1()
{
	//Checked 16/07/2021
	//Exception not supported by PSX Bios
	//Does it throw exceptions?
	//CauseRegister CE field must be set to 1 to trigger this exception?
	//exception(static_cast<uint32_t>(cpu::exceptionCause::copunusable));

	return true;
}

bool CpuShort::op_swc2()
{
	//Checked 31/03/2023
	//Does it throw exceptions?
	//CauseRegister CE field must be set to 2 to trigger this exception?
	wrMem(currentOpcode.regA + currentOpcode.imm, cop2->reg.data[currentOpcode.rt], 4);
		
	return true;
}

bool CpuShort::op_swc3()
{
	//Checked 16/07/2021
	//Exception not supported by PSX Bios
	//Does it throw exceptions?
	//CauseRegister CE field must be set to 3 to trigger this exception?
	//exception(static_cast<uint32_t>(cpu::exceptionCause::copunusable));

	return true;
}

bool CpuShort::op_sll()
{
	uint32_t result = currentOpcode.regB << currentOpcode.shamt;

	if (currentOpcode.rd != 0)
		writeRegister(currentOpcode.rd, result);

	return true;
}

bool CpuShort::op_srl()
{
	uint32_t result = currentOpcode.regB >> currentOpcode.shamt;

	if (currentOpcode.rd != 0)
		writeRegister(currentOpcode.rd, result);

	return true;
}

bool CpuShort::op_sra()
{
	uint32_t result = (int32_t)currentOpcode.regB >> currentOpcode.shamt;

	if (currentOpcode.rd != 0)
		writeRegister(currentOpcode.rd, result);

	return true;
}

bool CpuShort::op_sllv()
{
	uint32_t result = currentOpcode.regB << (currentOpcode.regA & 0x0000001f);

	if (currentOpcode.rd != 0)
		writeRegister(currentOpcode.rd, result);

	return true;
}

bool CpuShort::op_srlv()
{
	uint32_t result = currentOpcode.regB >> (currentOpcode.regA & 0x0000001f);

	if (currentOpcode.rd != 0)
		writeRegister(currentOpcode.rd, result);

	return true;
}

bool CpuShort::op_srav()
{
	uint32_t result = (int32_t)currentOpcode.regB >> (currentOpcode.regA & 0x0000001f);

	if (currentOpcode.rd != 0)
		writeRegister(currentOpcode.rd, result);

	return true;
}

bool CpuShort::op_jr()
{
	//Check Target Address Alignment, last two bits must be 0
	uint32_t targetAddress = currentOpcode.regA;

	//Check Target Address Alignment, last two bits must be 0
	if (targetAddress & 0x3)
	{
		LOG_F(ERROR, "CPU - Address Load Error: 0x%08x jr r%d  - [0x%08x]", pc - 4, currentOpcode.rs, currentOpcode.regA);
		
		cop0->reg[8] = targetAddress;
		exception(static_cast<uint32_t>(cpu::exceptionCause::addrerrload));
		
		return true;
	}

	//Set branchAddress
	branchAddress = targetAddress;
	branchFunctionAddress = pc - 4;
	isInDelaySlot = true;

	return true;
}

bool CpuShort::op_jalr()
{
	uint32_t targetAddress = currentOpcode.regA;
	
	//Check Target Address Alignment, last two bits must be 0
	if (targetAddress & 0x3)
	{
		LOG_F(ERROR, "CPU - Address Load Error: 0x%08x jalr r%d, r%d  - [0x%08x], [0x%08x]", pc - 4, currentOpcode.rd, currentOpcode.rs, gpr[currentOpcode.rd], currentOpcode.regA);
		
		//Return Address is set even if instruction cause an exception
		//Should return to the istruction after the delay slot instruction. PC already point at the delay slot at this stage
		if (currentOpcode.rd != 0)
			writeRegister(currentOpcode.rd, pc + 4);

		cop0->reg[8] = targetAddress;
		exception(static_cast<uint32_t>(cpu::exceptionCause::addrerrload));

		return true;
	}

	//Set branchAddress
    branchAddress = targetAddress;
	branchFunctionAddress = pc - 4;
	isInDelaySlot = true;

	//Set Return Address
	//Should return to the istruction after the delay slot instruction. PC already point at the delay slot at this stage
	if (currentOpcode.rd != 0)
		writeRegister(currentOpcode.rd, pc + 4);

	return true;
}

bool CpuShort::op_syscall()
{
	exception(static_cast<uint32_t>(cpu::exceptionCause::syscall));

	return true;
}

bool CpuShort::op_break()
{
	//Exception not supported by PSX Bios
	exception(static_cast<uint32_t>(cpu::exceptionCause::breakpoint));

	return true;
}

bool CpuShort::op_mfhi()
{
	if (currentOpcode.rd != 0)
		writeRegister(currentOpcode.rd, hi);
	
	return true;
}

bool CpuShort::op_mthi()
{
	hi = currentOpcode.regA;

	return true;
}

bool CpuShort::op_mflo()
{
	if (currentOpcode.rd != 0)
		writeRegister(currentOpcode.rd, lo);

	return true;
}

bool CpuShort::op_mtlo()
{
	lo = currentOpcode.regA;

	return true;
}

bool CpuShort::op_mult()
{
	int64_t a, b, r;

	a = (int64_t)(int32_t)currentOpcode.regA;
	b = (int64_t)(int32_t)currentOpcode.regB;
	r = a * b;
	hi = (uint32_t)(r >> 32);
	lo = (uint32_t)r;

	return true;
}

bool CpuShort::op_multu()
{
	uint64_t a, b, r;

	a = (uint64_t)currentOpcode.regA;
	b = (uint64_t)currentOpcode.regB;
	r = a * b;
	hi = (uint32_t)(r >> 32);
	lo = (uint32_t)r;

	return true;
}

bool CpuShort::op_div()
{
    int32_t n = currentOpcode.regA;
    int32_t d = currentOpcode.regB;

    if (d == 0)
	{
        hi = (uint32_t)n;
        if (n >= 0)
		{
            lo = 0xffffffff;
        } 
		else
		{
            lo = 0x1;
        }
    }
	else if (((uint32_t)n) == 0x80000000 && ((uint32_t)d) == 0xffffffff)
	{
        hi = 0;
        lo = 0x80000000;
    } 
	else
	{
        hi = ((uint32_t)(n % d));
        lo = ((uint32_t)(n / d));
    }

	return true;
}

bool CpuShort::op_divu()
{
    uint32_t n = currentOpcode.regA;
    uint32_t d = currentOpcode.regB;

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

bool CpuShort::op_add()
{
	int32_t s = (int32_t)currentOpcode.regA;
    int32_t t = (int32_t)currentOpcode.regB;
    int32_t r = s + t;
	
	// Overflow occurs if the sign bits of the inputs are the same and the sign bit of the result is different
    if (((s ^ r) & (t ^ r)) < 0)
    {
        exception(static_cast<uint32_t>(cpu::exceptionCause::overflow));
        return true;
    }
    else
    {
        if (currentOpcode.rd != 0)
			writeRegister(currentOpcode.rd, (uint32_t)r);
    }

    return true;
}

bool CpuShort::op_addu()
{
	uint32_t result = (uint32_t)currentOpcode.regA + (uint32_t)currentOpcode.regB;

	if (currentOpcode.rd != 0)
		writeRegister(currentOpcode.rd, result);

	return true;
}

bool CpuShort::op_sub()
{
	uint32_t s = currentOpcode.regA;
    uint32_t t = currentOpcode.regB;
    uint32_t r = s - t;

    // Two's-complement overflow if carries out of bits 30 and 31 differ:
    if ((((s ^ t) & (s ^ r)) & 0x80000000U) != 0)
    {
        exception(static_cast<uint32_t>(cpu::exceptionCause::overflow));
        return true;
    }
    else
    {
        if (currentOpcode.rd != 0)
			writeRegister(currentOpcode.rd, r);
    }

    return true;
}

bool CpuShort::op_subu()
{
	uint32_t result = (uint32_t)currentOpcode.regA - (uint32_t)currentOpcode.regB;

	if (currentOpcode.rd != 0)
		writeRegister(currentOpcode.rd, result);

	return true;
}

bool CpuShort::op_and()
{
	uint32_t result = currentOpcode.regA & currentOpcode.regB;

	if (currentOpcode.rd != 0)
		writeRegister(currentOpcode.rd, result);

	return true;
}

bool CpuShort::op_or()
{
	uint32_t result = currentOpcode.regA | currentOpcode.regB;

	if (currentOpcode.rd != 0)
		writeRegister(currentOpcode.rd, result);

	return true;
}

bool CpuShort::op_xor()
{
	uint32_t result = currentOpcode.regA ^ currentOpcode.regB;

	if (currentOpcode.rd != 0)
		writeRegister(currentOpcode.rd, result);

	return true;
}

bool CpuShort::op_nor()
{
	uint32_t result = ~(currentOpcode.regA | currentOpcode.regB);

	if (currentOpcode.rd != 0)
		writeRegister(currentOpcode.rd, result);

	return true;
}

bool CpuShort::op_slt()
{
	uint32_t result = ((int32_t)currentOpcode.regA < (int32_t)currentOpcode.regB) ? 0x00000001 : 0x00000000;

	if (currentOpcode.rd != 0)
		writeRegister(currentOpcode.rd, result);

	return true;
}

bool CpuShort::op_sltu()
{
	uint32_t result = ((uint32_t)currentOpcode.regA < (uint32_t)currentOpcode.regB) ? 0x00000001 : 0x00000000;

	if (currentOpcode.rd != 0)
		writeRegister(currentOpcode.rd, result);

	return true;
}

bool CpuShort::op_unknown()
{
	//Exception not supported by PSX Bios
	//exception(static_cast<uint32_t>(cpu::exceptionCause::resinst));
	return false;
}

//-----------------------------------------------------------------------------------------------------------------------------------
// Setters and Getters
//-----------------------------------------------------------------------------------------------------------------------------------
uint32_t CpuShort::get_pc() const
{
    return pc;
}

void CpuShort::set_pc(uint32_t value)
{
}

uint32_t CpuShort::get_hi() const
{
    return hi;
}

void CpuShort::set_hi(uint32_t value)
{
	hi = value;
}

uint32_t CpuShort::get_lo() const
{
    return lo;
}

void CpuShort::set_lo(uint32_t value)
{
	lo = value;
}

uint32_t CpuShort::get_gpr(uint8_t regNum) const
{
    return gpr[regNum];
}

void CpuShort::set_gpr(uint8_t regNum, uint32_t value)
{
	if (regNum != 0)
		gpr[regNum] = value;
}

