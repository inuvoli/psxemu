#pragma once

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
#include <map>
#include <memory>

#include "litelib.h" 

#include "cpu_registers.h"
#include "cop0.h"
#include "cop2.h"


class Psx;

//DebugInfo
struct callstackinfo
{
	uint32_t		jumpaddr;
	uint32_t		pc;
	uint32_t		sp;
	uint32_t		ra;
	std::string		func;
};

struct decodedOpcode
{
	uint32_t	regA;	//Content of rs (aka base)
	uint32_t	regB;	//Content of rt
	uint32_t	imm;	//32bit Sign extendend immediate value
	uint8_t		rs;
	uint8_t		rd;
	uint8_t		rt;
	uint8_t		op;
	uint8_t		funct;
	uint32_t	tgt;
	uint8_t		shamt;
	uint32_t	cofun;
};

class CPU
{
public:
	CPU();
	~CPU();

	bool reset();
	bool execute();

	//Cache & Memory Access
	uint32_t rdInst(uint32_t vAddr, uint8_t bytes = 4);
	uint32_t rdMem(uint32_t vAddr, uint8_t bytes = 4);
	bool	 wrMem(uint32_t vAddr, uint32_t& data, uint8_t bytes = 4, bool checkalign = true);
	uint32_t rdDataCache(uint32_t vAddr, uint8_t bytes);
	bool	 wrDataCache(uint32_t vAddr, uint32_t& data, uint8_t bytes);
	uint32_t rdInstrCache(uint32_t vAddr, uint8_t bytes);
	bool	 wrInstrCache(uint32_t vAddr, uint32_t& data, uint8_t bytes);

	//Pipeline Fase Functions
	bool exception(uint32_t cause);
	bool interrupt(uint8_t status);

public:
	//CPU Internal Registers
	uint32_t	pc;					//Program Counter
	uint32_t	hi;					//HI Register, used for Mult and Div
	uint32_t	lo;					//LO Register, used for Mult and Div
	uint32_t	gpr[32];			//CPU General Purpose Registers
	uint32_t	cacheReg;			//Cache Control Register (mapped to 0xfffe0130)

	//DEBUG - Call Stack
	lite::circularbuffer<callstackinfo, 31>	callStack;
	
	//Coprocessors
	std::shared_ptr<Cop0>	cop0;	//Coprocessor 0
	std::shared_ptr<Cop2>	cop2;	//Coprocessor 2 (GTE)

	//Pipeline Registers and Status
	decodedOpcode	currentOpcode;
	bool			branchDelaySlot;
	uint32_t		branchAddress;
	bool			stallPipeline;
	bool			dmaTakeOnBus;

	//Instruction & Data Cache
	uint8_t		iCache[ICACHE_SIZE];
	uint8_t		dCache[DCACHE_SIZE];  //ScratchPad, accessed at address 0x1f800000 - 0x1f8003ff

	//Instruction & Data Cache Status
	bool iCacheEnabled;
	bool dCacheEnabled;
	
	//Connect to PSX Instance
	void link(Psx* instance) { psx = instance; }

private:
	//Helper functions
	bool isOverflow(int32_t a, int32_t b);

	//CPU Instructions
	bool op_bxx();
	bool op_j();
	bool op_jal();
	bool op_beq();
	bool op_bne();
	bool op_blez();
	bool op_bgtz();
	bool op_addi();
	bool op_addiu();
	bool op_slti();
	bool op_sltiu();
	bool op_andi();
	bool op_ori();
	bool op_xori();
	bool op_lui();
	bool op_cop0();
	bool op_cop1();
	bool op_cop2();
	bool op_cop3();
	bool op_lb();
	bool op_lh();
	bool op_lwl();
	bool op_lw();
	bool op_lbu();
	bool op_lhu();
	bool op_lwr();
	bool op_sb();
	bool op_sh();
	bool op_swl();
	bool op_sw();
	bool op_swr();
	bool op_lwc0();
	bool op_lwc1();
	bool op_lwc2();
	bool op_lwc3();
	bool op_swc0();
	bool op_swc1();
	bool op_swc2();
	bool op_swc3();

	//CPU Functions
	bool op_sll();
	bool op_srl();
	bool op_sra();
	bool op_sllv();
	bool op_srlv();
	bool op_srav();
	bool op_jr();
	bool op_jalr();
	bool op_syscall();
	bool op_break();
	bool op_mfhi();
	bool op_mthi();
	bool op_mflo();
	bool op_mtlo();
	bool op_mult();
	bool op_multu();
	bool op_div();
	bool op_divu();
	bool op_add();
	bool op_addu();
	bool op_sub();
	bool op_subu();
	bool op_and();
	bool op_or();
	bool op_xor();
	bool op_nor();
	bool op_slt();
	bool op_sltu();

	//Unknown OpCode
	bool op_unknown();

private:
	//Link to Bus Object
	Psx* psx;

	//Full set of CPU Instruction Dictionaries
	struct INSTR
	{
		std::string mnemonic;
		bool(CPU::* operate)() = nullptr;
	};

	std::vector<INSTR> instrSet;		//Full Instruction Set
	std::vector<INSTR> functSet;		//Full Function Set

	//Memory Mapping
	lite::range memRangeScratchpad =  lite::range(0x1f800000, 0x400);
	lite::range memRangeIntRegs = lite::range(0x1f801070, 0x8);
	lite::range memRangeCacheRegs = lite::range(0xfffe0130, 0x4);

	static constexpr uint32_t	lwlMask[4] = { 0x00ffffff, 0x0000ffff, 0x000000ff, 0x00000000 };
	static constexpr uint32_t	lwrMask[4] = { 0x00000000, 0xff000000, 0xffff0000, 0xffffff00 };
};

