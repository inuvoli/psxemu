#include <loguru.hpp>
#include "memory.h"

Memory::Memory()
{
	ramSize = 0x0;
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

	return true;
}

uint32_t Memory::read(uint32_t phAddr, uint8_t bytes)
{
	uint32_t data = 0;
		
	for (int i = 0; i < bytes; i++)
	{
		data += (ram[phAddr + i] << (8 * i));
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
