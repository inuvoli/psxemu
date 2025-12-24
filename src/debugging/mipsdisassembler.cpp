#include "mipsdisassembler.h"

MipsDisassembler::MipsDisassembler()
{
	instrSet =
	{
		{"special", false},
		{"bxx rs, imm", false},
		{"j tgt", true},
		{"jal tgt", true},
		{"beq rs, rt, imm", false},
		{"bne rs, rt, imm", false},
		{"blez rs, imm", false},
		{"bgtz rs, imm", false},
		{"addi rt, rs, imm", false},
		{"addiu rt, rs, imm", false},
		{"slti rt, rs, imm", false},
		{"sltiu rt, rs, imm", false},
		{"andi rt, rs, imm", false},
		{"ori rt, rs, imm", false},
		{"xori rt, rs, imm", false},
		{"lui rt, imm", false},
		{"cop0 cofun", false},
		{"cop1 cofun", false},
		{"cop2 cofun", false},
		{"cop3 cofun", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"lb rt, imm(rs)", false},
		{"lh rt, imm(rs)", false},
		{"lwl rt, imm(rs)", false},
		{"lw rt, imm(rs)", false},
		{"lbu rt, imm(rs)", false},
		{"lhu rt, imm(rs)", false},
		{"lwr rt, imm(rs)", false},
		{"invalid operation", false},
		{"sb rt, imm(rs)", false},
		{"sh rt, imm(rs)", false},
		{"swl rt, imm(rs)", false},
		{"sw rt, imm(rs)", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"swr rt, imm(rs)", false},
		{"invalid operation", false},
		{"lwc0 rt, imm(rs)", false},
		{"lwc1 rt, imm(rs)", false},
		{"lwc2 rt, imm(rs)", false},
		{"lwc3 rt, imm(rs)", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"swc0 rt, imm(rs)", false},
		{"swc1 rt, imm(rs)", false},
		{"swc2 rt, imm(rs)", false},
		{"swc3 rt, imm(rs)", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false}
	};

	functSet =
	{
		{"sll rd, rt, shamt", false},
		{"invalid operation", false},
		{"srl rd, rt, shamt", false},
		{"sra rd,rt, shamt", false},
		{"sllv rd, rt, rs", false},
		{"invalid operation", false},
		{"srlv rd, rt, rs", false},
		{"srav rd, rt, rs", false},
		{"jr rs", true},
		{"jalr rd, rs", true},
		{"invalid operation", false},
		{"invalid operation", false},
		{"syscall", false},
		{"break", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"mfhi rd", false},
		{"mthi rs", false},
		{"mflo rd", false},
		{"mtlo rs", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"mult rs, rt", false},
		{"multu rs, rt", false},
		{"div rs, rt", false},
		{"divu rs, rt", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"add rd, rs, rt", false},
		{"addu rd, rs, rt", false},
		{"sub rd, rs, rt", false},
		{"subu rd, rs, rt", false},
		{"and rd, rs, rt", false},
		{"or rd, rs, rt", false},
		{"xor rd, rs, rt", false},
		{"nor rd, rs, rt", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"slt rd, rs, rt", false},
		{"sltu rd, rs, rt", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false},
		{"invalid operation", false}
	};
}

MipsDisassembler::~MipsDisassembler()
{

}

//-----------------------------------------------------------------------------------------------------------------------------------
//
// MIPS CPU DISASSEMBLER FUNCTIONS
//
//-----------------------------------------------------------------------------------------------------------------------------------
AsmCode MipsDisassembler::disassemble(const uint8_t* rom, const uint8_t* ram, uint32_t startAddr, uint32_t stopAddr)
{
	std::string	asmLine;
	std::string asmLabel;
	uint32_t currentAddr;
	uint32_t opcode;
	AsmCode asmCode;
	bool isJump;

	currentAddr = startAddr;
	
	while (currentAddr <= stopAddr)
	{
		auto res = decodeAddress(currentAddr);

		if (res.second)
		{
			//Address is in ROM
			opcode = rom[res.first] + (rom[res.first + 1] << 8) + (rom[res.first + 2] << 16) + (rom[res.first + 3] << 24);
		}
		else
		{
			//Address is in RAM
			opcode = ram[res.first] + (ram[res.first + 1] << 8) + (ram[res.first + 2] << 16) + (ram[res.first + 3] << 24);
		}
		
		asmLine = decodeOpcode(opcode, isJump);
		asmLabel = ""; //TODO
		asmCode.insert(std::make_pair(currentAddr, std::make_tuple(opcode, asmLabel, asmLine)));

		currentAddr += 4;
	}
	
	return asmCode;
}

AsmCode MipsDisassembler::disassemble(const uint8_t* rom, const uint8_t* ram, uint32_t startAddr, uint16_t maxInstructions)
{
	std::string	asmLine;
	std::string asmLabel;
	uint32_t currentAddr;
	uint32_t decodedInstr;
	uint32_t opcode;
	AsmCode asmCode;
	bool	isJump;
	bool	lastOpcode;

	currentAddr = startAddr;
	decodedInstr = 0;
	lastOpcode = false;

	while (decodedInstr <= maxInstructions)
	{
		auto res = decodeAddress(currentAddr);

		if (res.second)
		{
			//Address is in ROM
			opcode = rom[res.first] + (rom[res.first + 1] << 8) + (rom[res.first + 2] << 16) + (rom[res.first + 3] << 24);
		}
		else
		{
			//Address in in RAM
			opcode = ram[res.first] + (ram[res.first + 1] << 8) + (ram[res.first + 2] << 16) + (ram[res.first + 3] << 24);
		}

		asmLine = decodeOpcode(opcode, isJump);
		asmLabel = ""; //TODO
		asmCode.insert(std::make_pair(currentAddr, std::make_tuple(opcode, asmLabel, asmLine)));

		//Check if the last decoded instruction is a Jump instruction
		//Decode also the instruction in the Branch Delay Slot
		if (lastOpcode) break;
		if (isJump) lastOpcode = true;
		
		currentAddr += 4;
		decodedInstr++;
	}

	return asmCode;
}

bool MipsDisassembler::isJumpInstruction(const uint8_t* rom, const uint8_t* ram, uint32_t pc)
{
	uint32_t opcode;
	bool	isJump;

	auto res = decodeAddress(pc);

	if (res.second)
	{
		//Address is in ROM
		opcode = rom[res.first] + (rom[res.first + 1] << 8) + (rom[res.first + 2] << 16) + (rom[res.first + 3] << 24);
	}
	else
	{
		//Address in in RAM
		opcode = ram[res.first] + (ram[res.first + 1] << 8) + (ram[res.first + 2] << 16) + (ram[res.first + 3] << 24);
	}

	decodeOpcode(opcode, isJump);

    return isJump;
}

//-----------------------------------------------------------------------------------------------------------------------------------
// Private Helper Functions
//-----------------------------------------------------------------------------------------------------------------------------------
std::pair<uint32_t, bool> MipsDisassembler::decodeAddress(uint32_t vAddr)
{
	uint32_t phAddr = vAddr & regionMask[vAddr >> 29];
	bool isRom = (phAddr >= 0x1fc00000) && (phAddr <= 0x1fc70000);
	if (isRom) phAddr &= 0x0007ffff;
	auto res = std::make_pair(phAddr, isRom);
	return res;
}

std::string MipsDisassembler::decodeOpcode(uint32_t data, bool& isJump)
{

	auto cond = [](uint8_t flag)
	{
		std::string condition;

		if (flag == 0x00)
			condition = "ltz";

		if (flag == 0x01)
			condition = "gez";

		if (flag == 0x10)
			condition = "ltzal";

		if (flag == 0x11)
			condition = "gezal";

		return condition;
	};

	auto hex = [](uint32_t n, uint8_t d)
	{
		std::string s(d, '0');
		for (int i = d - 1; i >= 0; i--, n >>= 4)
			s[i] = "0123456789abcdef"[n & 0xF];
		return s;
	};
	
	std::string asmLine = "";
	Instruction opcode;
	size_t	strPos;
	
	//Decode Opcode
	opcode.word = data;

	//Compose full Assembler Line replacing all placeholder with actual values 	
	switch (opcode.op)
	{
	case 0x00:		//Special Function
		asmLine += functSet[opcode.funct].mnemonic;
		isJump = functSet[opcode.funct].isjump;

		break;

	case 0x10:		//Coprocessor0 Functions
		if (opcode.cop)
		{
			switch (opcode.funct)
			{
			case 0x01:
				asmLine += "tlbr";
				break;
			case 0x02:
				asmLine += "tlbwi";
				break;
			case 0x06:
				asmLine += "tlbwr";
				break;
			case 0x08:
				asmLine += "tlbp";
				break;
			case 0x10:
				asmLine += "rfe";
				break;
			}
		}
		else
		{
			switch (opcode.rs)
			{
			case 0x00:
				asmLine += "mfc0 rt, rd";
				break;
			case 0x02:
				asmLine += "cfc0 rt, rd";
				break;
			case 0x04:
				asmLine += "mtc0 rt, rd";
				break;
			case 0x06:
				asmLine += "ctc0 rt, rd";
				break;
			}
		}
		break;

	case 0x11:		//Coprocessor1 Functions
		if (opcode.cop)
			asmLine += instrSet[opcode.op].mnemonic;
		else
		{
			switch (opcode.rs)
			{
			case 0x00:
				asmLine += "mfc1 rt, rd";
				break;
			case 0x02:
				asmLine += "cfc1 rt, rd";
				break;
			case 0x04:
				asmLine += "mtc1 rt, rd";
				break;
			case 0x06:
				asmLine += "ctc1 rt, rd";
				break;
			}
		}
		break;

	case 0x12:		//Coprocessor2 Functions
		if (opcode.cop)
			asmLine += instrSet[opcode.op].mnemonic;
		else
		{
			switch (opcode.rs)
			{
			case 0x00:
				asmLine += "mfc2 rt, rd";
				break;
			case 0x02:
				asmLine += "cfc2 rt, rd";
				break;
			case 0x04:
				asmLine += "mtc2 rt, rd";
				break;
			case 0x06:
				asmLine += "ctc2 rt, rd";
				break;
			}
		}
		break;

	case 0x13:		//Coprocessor3 Functions
		if (opcode.cop)
			asmLine += instrSet[opcode.op].mnemonic;
		else
		{
			switch (opcode.rs)
			{
			case 0x00:
				asmLine += "mfc3 rt, rd";
				break;
			case 0x02:
				asmLine += "cfc3 rt, rd";
				break;
			case 0x04:
				asmLine += "mtc3 rt, rd";
				break;
			case 0x06:
				asmLine += "ctc3 rt, rd";
				break;
			}
		}
		break;

	default:		//All Other Functions
		asmLine += instrSet[opcode.op].mnemonic;
		isJump = instrSet[opcode.op].isjump;
		break;
	}

	//Replace Placeholders
	strPos = asmLine.find("xx");
	if (opcode.op == 0x1 && strPos != std::string::npos)
		asmLine.replace(strPos, 2, cond(opcode.rt));

	strPos = asmLine.find("rs");
	if (strPos != std::string::npos)
		asmLine.replace(strPos, 2, cpuRegisterName[opcode.rs]);

	strPos = asmLine.find("rt");
	if (strPos != std::string::npos)
		asmLine.replace(strPos, 2, cpuRegisterName[opcode.rt]);

	strPos = asmLine.find("rd");
	if (strPos != std::string::npos)
		asmLine.replace(strPos, 2, cpuRegisterName[opcode.rd]);

	strPos = asmLine.find("imm");
	if (strPos != std::string::npos)
		asmLine.replace(strPos, 3, "0x" + hex(opcode.imm, 4));

	strPos = asmLine.find("shamt");
	if (strPos != std::string::npos)
		asmLine.replace(strPos, 5, "0x" + hex(opcode.shamt, 2));

	strPos = asmLine.find("tgt");
	if (strPos != std::string::npos)
		asmLine.replace(strPos, 3, "0x" + hex(opcode.tgt, 7));

	strPos = asmLine.find("cofun");
	if (strPos != std::string::npos)
		asmLine.replace(strPos, 5, "0x" + hex(opcode.cofun, 7));
	
	return asmLine;
}

