#include <loguru.hpp>
#include "bios.h"

Bios::Bios()
{
}

Bios::~Bios()
{
}

bool Bios::loadBios(const std::string& fileName)
{
	//Load BIOS Image
	LOG_F(INFO, "PSP Bios (%s) Loading...", fileName.c_str());

	std::ifstream ifs;
	ifs.open(fileName, std::ifstream::binary);
	if (ifs.is_open())
	{
		ifs.seekg(0, ifs.end);
		uint32_t biosSize = (uint32_t)ifs.tellg();
		ifs.seekg(0, ifs.beg);

		if (biosSize != BIOS_SIZE)
		{
			LOG_F(ERROR, "PSP Bios Unknown!");
			return false;
		}

		ifs.read((char*)rom, biosSize);

		ifs.close();

		LOG_F(INFO, "PSP Bios Loaded");

#ifdef PSXEMU_ENABLE_STDIO
		uint32_t patch[] = { 0x01, 0x00, 0x01, 0x24, 0xe1, 0x19, 0xf0, 0x0f, 0xc0, 0xa9, 0x81, 0xaf };
		for(int i=0;i<12;i++)
			rom[0x06f0c + i] = patch[i];
		LOG_F(INFO, "PSP Bios Patched - Enable std_io");
#endif

		return true;
	}
	LOG_F(ERROR, "PSP Bios Not Found");

	return false;
}

uint32_t Bios::read(uint32_t phAddr, uint8_t bytes)
{
	uint32_t data = 0;
	uint32_t addr = phAddr & 0x0007ffff;

	for (int i = 0; i < bytes; i++)
	{
		data += (rom[addr + i] << (8 * i));
	}
		
	return data;
}
