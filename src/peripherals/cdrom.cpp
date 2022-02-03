#include "cdrom.h"

Cdrom::Cdrom()
{
}

Cdrom::~Cdrom()
{
}

bool Cdrom::reset()
{
	return false;
}

bool Cdrom::clock()
{
	return false;
}

bool Cdrom::setParameter(uint32_t addr, uint32_t& data, uint8_t bytes)
{
	switch (addr)
	{

	default:
		printf("CDROM - Unknown Parameter Set addr: 0x%08x (%d), data: 0x%08x\n", addr, bytes, data);
		return false;
	}
	return true;
}

uint32_t Cdrom::getParameter(uint32_t addr, uint8_t bytes)
{
	uint32_t data;

	switch (addr)
	{

	default:
		printf("CDROM - Unknown Parameter Get addr: 0x%08x (%d)\n", addr, bytes);
		return 0x8;
	}

	return data;
}


