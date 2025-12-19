#include <loguru.hpp>
#include "memory.h"

Memory::Memory()
{
	ramSize = 0x0;

	//Reset Executable Informations
	exeInfo.isPresent = false;
	exeInfo.regPCValue = 0x0;
	exeInfo.reg28Value = 0x0;
	exeInfo.reg29Value = 0x0;
	exeInfo.reg30Value = 0x0;
}

Memory::~Memory()
{
}

bool Memory::reset()
{
	//Reset RAM Content
	memset(ram, 0x00, sizeof(ram));

	//Reset Internal Registers
	ramSize = 0x0;

	//Reset Executable Informations
	exeInfo.isPresent = false;
	exeInfo.regPCValue = 0x0;
	exeInfo.reg28Value = 0x0;
	exeInfo.reg29Value = 0x0;
	exeInfo.reg30Value = 0x0;

	return true;
}

bool Memory::loadExe(const std::string& fileName)
{
	uint32_t destVirtualAddress;
	uint32_t destPhisicalAddress;
	uint32_t fileSize;

	//Load EXE Image
	LOG_F(INFO, "PSP EXE (%s) Loading...", fileName.c_str());

	std::ifstream exeFile(fileName, std::ios::in | std::ios::binary | std::ios::ate);
	if (exeFile.is_open())
	{
		int size = (int)exeFile.tellg();
		uint8_t* exeData = new uint8_t[size];
		exeFile.seekg(0, std::ios::beg);
		exeFile.read((char*)exeData, size);
		exeFile.close();
		LOG_F(INFO, "PSP EXE Loaded");

		const char correctHeader[] = "PS-X EXE";
		for (int i = 0; i < sizeof(correctHeader); i++)
		{
			if (exeData[i] != correctHeader[i])
			{
				LOG_F(ERROR, "PSP EXE has incorrect header");
				delete[] exeData;
				return false;
			}
		}
		LOG_F(INFO, "PSP EXE header detected");

		exeInfo.isPresent = true;

		uint32_t offset = exeData[0x34] | (((uint32_t)exeData[0x35]) << 8) | (((uint32_t)exeData[0x36]) << 16) | (((uint32_t)exeData[0x37]) << 24);

		exeInfo.regPCValue = exeData[0x10] | (((uint32_t)exeData[0x11]) << 8) | (((uint32_t)exeData[0x12]) << 16) | (((uint32_t)exeData[0x13]) << 24);
		exeInfo.reg28Value = exeData[0x14] | (((uint32_t)exeData[0x15]) << 8) | (((uint32_t)exeData[0x16]) << 16) | (((uint32_t)exeData[0x17]) << 24);
		exeInfo.reg29Value = exeData[0x30] | (((uint32_t)exeData[0x31]) << 8) | (((uint32_t)exeData[0x32]) << 16) | (((uint32_t)exeData[0x33]) << 24);
		exeInfo.reg29Value = exeInfo.reg29Value + offset;
		exeInfo.reg30Value = exeInfo.reg29Value;
		
		destVirtualAddress = exeData[0x18] | (((uint32_t)exeData[0x19]) << 8) | (((uint32_t)exeData[0x1A]) << 16) | (((uint32_t)exeData[0x1B]) << 24);
		fileSize = exeData[0x1C] | (((uint32_t)exeData[0x1D]) << 8) | (((uint32_t)exeData[0x1E]) << 16) | (((uint32_t)exeData[0x1F]) << 24);

		LOG_F(INFO, "PC    Value [0x%08x]", exeInfo.regPCValue);
		LOG_F(INFO, "Reg28 Value [0x%08x]", exeInfo.reg28Value);
		LOG_F(INFO, "Reg29 Value [0x%08x]", exeInfo.reg29Value);
		LOG_F(INFO, "Reg30 Value [0x%08x]", exeInfo.reg30Value);
		LOG_F(INFO, "EXE Location [0x%08x], EXE Size [0x%08x]", destVirtualAddress, fileSize);

		//Copy EXE to Memory
		utility::Virtual2PhisicalAddr(destVirtualAddress, destPhisicalAddress);
		memcpy(ram + destPhisicalAddress, exeData + 0x800, fileSize);
		
		delete[] exeData;
		return true;
	}
	else
	{
		LOG_F(ERROR, "PSP EXE Not Found");
		return false;
	}
}

uint32_t Memory::read(uint32_t phAddr, uint8_t bytes)
{
	uint32_t data = 0;
		
	for (int i = 0; i < bytes; i++)
	{
		data |= (((uint32_t)ram[phAddr + i]) << (8 * i));
	}

	return data;
}

bool Memory::write(uint32_t phAddr, uint32_t& data, uint8_t bytes)
{
	for (int i = 0; i < bytes; i++)
	{
		ram[phAddr + i] = (uint8_t)(data >> (8 * i));
	}
	
	return true;
}

bool Memory::writeAddr(uint32_t addr, uint32_t& data, uint8_t bytes)
{
	switch (addr)
	{
	case 0x1f801060:
		ramSize = data;
		break;
	default:
		LOG_F(ERROR, "Memory - Unknown Parameter Set 0x%08x (%d)", addr, bytes);
		return false;
	}
	return true;
}

uint32_t Memory::readAddr(uint32_t addr, uint8_t bytes)
{
	LOG_F(ERROR, "Memory - Unknown Parameter Get 0x%08x (%d)", addr, bytes);
	return 0;
}
