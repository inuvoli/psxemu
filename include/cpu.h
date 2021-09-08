#pragma once

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
#include <map>

#include "bitfield.h"

//CPU Constant Definitions
 constexpr auto ICACHE_SIZE = 0x1000;				//4 KB
 constexpr auto DCACHE_SIZE = 0x0400;				//1 KB
 constexpr auto DCACHE_EN1_MASK =	0x00000008;
 constexpr auto DCACHE_EN2_MASK =	0x00000080;
 constexpr auto ICACHE_EN_MASK =	0x00000800;

 //constexpr auto CP0_SR_INT_MASK = 0x0000003f;
 //constexpr auto CP0_SR_ISC_MASK = 0x00000001 << 16;
 //constexpr auto CP0_SR_SWC_MASK = 0x00000001 << 17;
 //constexpr auto CP0_SR_BEV_MASK = 0x00000001 << 22;

 //constexpr auto CP0_CAUSE_EXCODE_MASK	= 0x0000007c;
 //constexpr auto CP0_CAUSE_BD_MASK		= 0x00000001 << 31;

// Decoding Unions
union Instruction
{
	uint32_t		word;

	BitField<26, 6> op;		//opcode
	BitField<21, 5> rs;		//rs index
	BitField<16, 5> rt;		//rt index
	BitField<11, 5> rd;		//rd index
	BitField<6, 5>	shamt;	//shift amount
	BitField<0, 6>	funct;	//function
	BitField<0, 16> imm;	//immediate value
	BitField<0, 26> tgt;	//target
	BitField<0, 25> cofun;	//coprocessor function
	BitField<25,1>	cop;	//cop	
};

union CauseRegister
{
	uint32_t		word;

	BitField<2, 5>	excode;	//exception code
	BitField<8, 8>	ip;		//Interrupt Pending
	BitField<28, 2>	ce;		//Coprocessor number exception
	BitField<31, 1>	bd;		//Branch Delay Exception
};

union StatusRegister
{
	uint32_t		word;

	BitField<0, 1>	iec;	//Current Interrupt enable
	BitField<1, 1>	kuc;	//Current Kernel/User mode
	BitField<2, 1>	iep;	//Previous Interrupt enable
	BitField<3, 1>	kup;	//Previous Kernel/User mode
	BitField<4, 1>	ieo;	//Old Interrupt enable
	BitField<5, 1>	kuo;	//Old Kernel/User mode
	BitField<8, 8>	im;		//Interrupt Mask
	BitField<16, 1>	isc;	//Isolate Cache
	BitField<17, 1>	swc;	//Swappe Cache mode
	BitField<18, 1>	pz;		//When set parity bit are written az 0's
	BitField<19, 1>	cm;		//Set if last access to D-Cache contained actual data
	BitField<20, 1>	pe;		//Cache parity check
	BitField<21, 1>	ts;		//TLB Shutdown
	BitField<22, 1>	bev;	//Boot Exception Vector (0 = RAM, 1 = ROM)
	BitField<25, 1>	re;		//Reverse Endianess (0 = normal endianess, 1 = 0 reversed endianess)
	BitField<28, 1>	cu0;	//COP0 Enable
	BitField<29, 1> cu1;	//COP1 Enable
	BitField<30, 1>	cu2;	//COP2 Enable
	BitField<31, 1>	cu3;	//COP3 Enable

	BitField<0, 6>	stkl;	//Access to Interrupt Enable / Kernel Mode Stack for left shifting
	BitField<0, 4>	stkr;	//Access to Interrupt Enable / Kernel Mode Stack for right shifting
};

//Pipeline Buffers, used to pass Signals and Data from one stage to the next
struct ifidBuffer
{
	uint32_t	pc;
	uint32_t	instr;
	bool		branchDelaySlot;
};

struct idexBuffer
{
	uint32_t	pc;		
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
	bool		cop;
	bool		branchDelaySlot;
};

struct exmemBuffer
{
	uint32_t	pc;
	uint32_t	aluRes;
	uint8_t		rd;
	uint32_t	regB;
	uint8_t		bytes;
	bool		readMem;
	bool		readMemUnaligned;
	bool		readMemSignExt;
	bool		writeMem;
	bool		writeMemUnaligned;
	bool		memToReg;
	bool		aluToReg;

};
struct memwbBuffer
{
	uint32_t	pc;
	uint32_t	memData;
	uint32_t	aluRes;
	uint8_t		rd;
	bool		memToReg;
	bool		aluToReg;
};

enum class exceptionCause
{
	interrupt =		0x00,
	addrerrload =	0x04,
	addrerrstore =	0x05,
	instrbuserr =	0x06,
	databusserr =	0x07,
	syscall =		0x08,
	breakpoint =	0x09,
	resinst =		0x0a, 
	copunusable =	0x0b,
	overflow =		0x0c
};

class Psx;

class CPU
{
public:
	CPU();
	~CPU();

	bool reset();
	bool clock();

	//Cache & Memory Access
	uint32_t rdInst(uint32_t vAddr, uint8_t bytes = 4);
	uint32_t rdMem(uint32_t vAddr, uint8_t bytes = 4, bool checkalign = true);
	bool	 wrMem(uint32_t vAddr, uint32_t& data, uint8_t bytes = 4, bool checkalign = true);
	uint32_t rdDataCache(uint32_t vAddr, uint8_t bytes);
	bool	 wrDataCache(uint32_t vAddr, uint32_t& data, uint8_t bytes);
	uint32_t rdInstrCache(uint32_t vAddr, uint8_t bytes);
	bool	 wrInstrCache(uint32_t vAddr, uint32_t& data, uint8_t bytes);

	//Pipeline Fase Functions
	bool fetch();
	bool decode();
	bool execute();
	bool memoryaccess();
	bool writeback();
	bool forwardCheck();
	bool exception(uint32_t cause);
	bool interrupt(uint32_t cause);

public:
	//CPU Internal Registers
	uint32_t	pc;				//Program Counter
	uint32_t	hi;				//HI Register, used for Mult and Div
	uint32_t	lo;				//LO Register, used for Mult and Div
	uint32_t	gpr[32];		//CPU General Purpose Registers
	uint32_t	cop0_reg[32];	//Coprocessor0 Registers
	uint32_t	cop1_reg[32];	//Coprocessor1 Registers (Not Implemented)
	uint32_t	cop2_reg[32];	//Coprocessor2 Registers
	uint32_t	cop3_reg[32];	//Coprocessor3 Registers (Not Implemented)
	uint32_t	cacheReg;		//Cache Control Register (mapped to 0xfffe0130)
	uint16_t	intStatus;		//Interrupt Status (mapped to 0x1f801070)
	uint16_t	intMask;		//Interrupt Mask (mapped to 0x1f801074)

	//Pipeline Registers and Status
	ifidBuffer	ifidReg;		//Write on Fetch, Read on Decode
	idexBuffer	idexReg;		//Write on Decode, Read on Execute
	exmemBuffer	exmemReg;		//Write on Execute, Read on MemoryAccess
	memwbBuffer	memwbReg;		//Write on MemoryAccess, Read on WriteBack
	bool		stallPipeline;
	bool		dmaTakeOnBus;

	//Instruction & Data Cache
	uint8_t		iCache[ICACHE_SIZE];
	uint8_t		dCache[DCACHE_SIZE];  //ScratchPad, accessed at address 0x1f800000 - 0x1f8003ff

	//Instruction & Data Cache Status
	bool iCacheEnabled;
	bool dCacheEnabled;
	
	//Connect to PSX Instance
	void link(Psx* instance) { psx = instance; }

	// Produces a map of strings, with keys equivalent to instruction start locations
	// in memory, for the specified address range
	std::map<uint32_t, std::string> disassemble(uint32_t nStart, uint32_t nStop);

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
	bool op_jarl();
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
		Psx* psx = nullptr;

		//Full set of CPU Instruction Dictionaries
		struct INSTR
		{
			std::string mnemonic;
			bool(CPU::* operate)() = nullptr;
		};

		std::vector<INSTR> instrSet;		//Full Instruction Set
		std::vector<INSTR> functSet;		//Full Function Set
};

