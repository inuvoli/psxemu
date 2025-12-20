#include "loguru.hpp"
#include "tty.h"
#include "psx.h"

Tty::Tty()
{
	buffer = {};
	bufferA = {};
	bufferB = {};

	ofs.open("ttydump.log");

}

Tty::~Tty()
{
	ofs.close();
}

bool Tty::reset()
{
	buffer = {};
	bufferA = {};
	bufferB = {};

	return true;
}

bool Tty::writeTTYChar(char c)
{
	buffer += c;
	if (c == '\n')
	{
		bufferA.emplace_back(buffer);
		buffer = "";
	}
	ofs << c;
	ofs.flush();
	return true;
}

bool Tty::writeAddr(uint32_t addr, uint32_t& data, uint8_t bytes)
{
	switch (addr)
	{
	case 0x1f802020:
		break;
    case 0x1f802021:
		break;
    case 0x1f802022:
		break;
    case 0x1f802023:	//Write Char on stdout Channel A
		buffer +=data;
		if (data == '\n')
		{
			bufferA.emplace_back(buffer);
			buffer = "";
		}
		ofs << static_cast<char>(data);
		ofs.flush();
		break;
    case 0x1f802024:
		break;
    case 0x1f802025:
		break;
    case 0x1f802026:
		break;
    case 0x1f802027:
		break;
    case 0x1f802028:
		break;
    case 0x1f802029:
		break;
    case 0x1f80202a:
		break;
    case 0x1f80202b:	//Write Char on stdout Channel B
		buffer +=data;
		if (data == '\n')
		{
			bufferB.emplace_back(buffer);
			buffer = "";
		}
		break;
    case 0x1f80202c:
		break;
    case 0x1f80202d:
		break;
    case 0x1f80202e:
		break;
    case 0x1f80202f:
		break;
	default:
		LOG_F(ERROR, "TTY - Unknown Parameter Set addr: 0x%08x (%d), data: 0x%08x", addr, bytes, data);
		return false;
	}
    LOG_F(3, "TTY - Parameter Set addr: 0x%08x (%d), data: 0x%08x", addr, bytes, data);

	return true;
}

uint32_t Tty::readAddr(uint32_t addr, uint8_t bytes)
{
	uint32_t data = 0;

	switch (addr)
	{
	case 0x1f802020:
		break;
    case 0x1f802021:
        data = 0x04;   //TX Ready Channel A
		break;
    case 0x1f802022:
		break;
    case 0x1f802023:
		break;
    case 0x1f802024:
		break;
    case 0x1f802025:
		break;
    case 0x1f802026:
		break;
    case 0x1f802027:
		break;
    case 0x1f802028:
		break;
    case 0x1f802029:
        data = 0x04;    //TX Ready Channel B
		break;
    case 0x1f80202a:
		break;
    case 0x1f80202b:
		break;
    case 0x1f80202c:
		break;
    case 0x1f80202e:
		break;
    case 0x1f80202f:
		break;
	default:
		printf("TTY - Unknown Parameter Get addr: 0x%08x (%d)\n", addr, bytes);
		return 0;
	}
	return data;
}

