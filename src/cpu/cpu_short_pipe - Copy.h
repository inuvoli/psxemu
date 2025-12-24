#pragma once

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
#include <map>
#include <memory>

#include "litelib.h" 
#include "cpu.h"

struct MemoryLoadDelay
{
	bool		pending;		//Set if there's a delayed memory load pending
	uint8_t		regIdentifier;	//Register Identifier
	uint32_t	regValue;		//Register Value	
};

struct decodedOpcode
{
	uint32_t	regA;	//Content of rs (aka base)
	uint32_t	regB;	//Content of rt
	uint32_t	imm;	//32bit Sign extendend immediate value
	uint8_t		rs;		//5-bit source register specifier
	uint8_t		rd;		//5-bit destination register specifier
	uint8_t		rt;		//5-bit target (source/destinatio) register specifier or branch condition
	uint8_t		op;		//6-bit operation code
	uint8_t		funct;	//6-bit function field
	uint32_t	tgt;	//26-bit target address
	uint8_t		shamt;	//5-bit shift amount
	uint32_t	cofun;	//26-bit coprocessor function
};

class CpuShort : public CPU
{
public:
	CpuShort();
	~CpuShort() override;

	bool reset() override;
	bool execute() override;

	//Cache & Memory Access
	uint32_t rdInst(uint32_t vAddr, uint8_t bytes = 4) override;
	uint32_t rdMem(uint32_t vAddr, uint8_t bytes = 4) override;
	bool	 wrMem(uint32_t vAddr, uint32_t& data, uint8_t bytes = 4, bool checkalign = true) override;
	uint32_t rdDataCache(uint32_t vAddr, uint8_t bytes) override;
	bool	 wrDataCache(uint32_t vAddr, uint32_t& data, uint8_t bytes) override;
	uint32_t rdInstrCache(uint32_t vAddr, uint8_t bytes) override;
	bool	 wrInstrCache(uint32_t vAddr, uint32_t& data, uint8_t bytes) override;

	//Pipeline Fase Functions
	bool exception(uint32_t cause) override;
	bool interrupt(uint8_t status) override;

	// Basic register accessors
	uint32_t get_pc() const override;
	void     set_pc(uint32_t value) override;
	uint32_t get_hi() const override;
	void     set_hi(uint32_t value) override;
	uint32_t get_lo() const override;
	void     set_lo(uint32_t value) override;
	uint32_t get_gpr(uint8_t regNumber) const override;
	void	 set_gpr(uint8_t regNumber, uint32_t value) override;
	
	//Pipeline Registers and Status
	decodedOpcode	currentOpcode;

	//Instruction & Data Cache
	uint8_t		iCache[ICACHE_SIZE];
	uint8_t		dCache[DCACHE_SIZE];  //ScratchPad, accessed at address 0x1f800000 - 0x1f8003ff

	//Instruction & Data Cache Status
	bool iCacheEnabled;
	bool dCacheEnabled;
	
	//Connect to PSX Instance
	void link(Psx* instance);

private:
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
	//Full set of CPU Instruction Dictionaries
	struct INSTR
	{
		std::string mnemonic;
		bool(CpuShort::* operate)() = nullptr;
	};

	MemoryLoadDelay		memoryLoadDelay;	//Used to managed Memory Load Delay

	std::vector<INSTR> instrSet;		//Full Instruction Set
	std::vector<INSTR> functSet;		//Full Function Set

	//Memory Mapping
	lite::range memRangeScratchpad =  lite::range(0x1f800000, 0x400);
	lite::range memRangeIntRegs = lite::range(0x1f801070, 0x8);
	lite::range memRangeCacheRegs = lite::range(0xfffe0130, 0x4);

	static constexpr uint32_t	lwlMask[4] = { 0x00ffffff, 0x0000ffff, 0x000000ff, 0x00000000 };
	static constexpr uint32_t	lwrMask[4] = { 0x00000000, 0xff000000, 0xffff0000, 0xffffff00 };
};

