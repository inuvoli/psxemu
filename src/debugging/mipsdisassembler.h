#pragma once

#include <cstdint>
#include <map>
#include <utility>
#include <tuple>
#include <string>
#include <vector>

#include "bitfield.h"

// Opcode, Label, Mnemonic
typedef std::map<uint32_t, std::tuple<uint32_t, std::string, std::string>> AsmCode;

class MipsDisassembler
{
public:
	MipsDisassembler();
	~MipsDisassembler();

	// Produces the code disassembly starting from from the startAddr to the first unconditional JUMP instruction.
	//  - binaryData: Pointer to struct containing raw binary data (RAM/ROM)
	//  - startAddr: starting disassenble address on the binary container
	//  - maxInstruction: maximum number of instruction to be decoded
	AsmCode disassemble(const uint8_t* rom, const uint8_t* ram, uint32_t startAddr, uint16_t maxInstructions = 200);

	// Produces the code disassembly from the startAddr to stopAddr.
	//  - binaryData: Pointer to struct containing raw binary data (RAM/ROM)
	//  - startAddr: starting disassemble address on the binary container
	//  - stopAddr: stopping disassemble address on the binary cointainer
	AsmCode disassemble(const uint8_t* rom, const uint8_t* ram, uint32_t startAddr, uint32_t stopAddr);

private:
	//Translate Virtual Address to Phisical Address and check if it is in ROM or RAM
	std::pair<uint32_t, bool> decodeAddress(uint32_t vAddr);

	//Decode single 32bit Opcode
	std::string decodeOpcode(uint32_t opcode, bool& isJump);

	//Memory Conversion Helper
	uint32_t	regionMask[8] = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x7fffffff, 0x1fffffff, 0xffffffff, 0xffffffff };

	// Opcode Decoding Union
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
		BitField<25, 1>	cop;	//cop	
	};

	//Full set of CPU Instruction Dictionaries
	struct INSTR
	{
		std::string mnemonic;
		bool isjump;
	};

	std::vector<INSTR> instrSet;		//Full Instruction Set
	std::vector<INSTR> functSet;		//Full Function Set

	//Register full names
	std::string cpuRegisterName[32] = { "zr", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
									"t8", "t9", "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "k0", "k1", "gp", "sp", "fp", "ra" };
};