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

		lite::bitfield<26, 6>	op;		//opcode
		lite::bitfield<21, 5>	rs;		//rs index
		lite::bitfield<16, 5>	rt;		//rt index
		lite::bitfield<11, 5>	rd;		//rd index
		lite::bitfield<6, 5>	shamt;	//shift amount
		lite::bitfield<0, 6>	funct;	//function
		lite::bitfield<0, 16>	imm;	//immediate value
		lite::bitfield<0, 26>	tgt;	//target
		lite::bitfield<0, 26>	cofun;	//coprocessor function
	};

	enum class exceptionCause : uint32_t
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

	enum class hwInterrupt : uint8_t
	{
		int0 = 0x01,
		int1 = 0x02,
		int2 = 0x04,
		int3 = 0x08,
		int4 = 0x10,
		int5 = 0x20
	};
};
