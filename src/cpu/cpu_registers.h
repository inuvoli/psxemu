#pragma once

//CPU Constant Definitions
constexpr auto ICACHE_SIZE = 0x1000;				//4 KB
constexpr auto DCACHE_SIZE = 0x0400;				//1 KB
constexpr auto DCACHE_EN1_MASK = 0x00000008;
constexpr auto DCACHE_EN2_MASK = 0x00000080;
constexpr auto ICACHE_EN_MASK = 0x00000800;

// Decoding Unions
namespace cpu
{
	union Instruction
	{
		uint32_t		word;

		lite::bitfield<26, 6> op;		//opcode
		lite::bitfield<21, 5> rs;		//rs index
		lite::bitfield<16, 5> rt;		//rt index
		lite::bitfield<11, 5> rd;		//rd index
		lite::bitfield<6, 5>	shamt;	//shift amount
		lite::bitfield<0, 6>	funct;	//function
		lite::bitfield<0, 16> imm;	//immediate value
		lite::bitfield<0, 26> tgt;	//target
		lite::bitfield<0, 26> cofun;	//coprocessor function
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
		interrupt = 0x00,
		addrerrload = 0x04,
		addrerrstore = 0x05,
		instrbuserr = 0x06,
		databusserr = 0x07,
		syscall = 0x08,
		breakpoint = 0x09,
		resinst = 0x0a,
		copunusable = 0x0b,
		overflow = 0x0c
	};
};
