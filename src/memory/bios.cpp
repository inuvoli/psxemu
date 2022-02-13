#include "bios.h"


Bios::Bios()
{
	//Load Bios
	loadBios("scph5501.bin");
}

Bios::~Bios()
{
}

bool Bios::loadBios(const std::string& fileName)
{
	//Load BIOS Image
	printf("PSP Bios (%s) Loading\n", fileName.c_str());

	std::ifstream ifs;
	ifs.open(fileName, std::ifstream::binary);
	if (ifs.is_open())
	{
		ifs.seekg(0, ifs.end);
		uint32_t biosSize = (uint32_t)ifs.tellg();
		ifs.seekg(0, ifs.beg);

		if (biosSize != BIOS_SIZE)
		{
			printf("PSP Bios Unknown\n");
			return false;
		}

		ifs.read((char*)rom, biosSize);

		ifs.close();

		printf("PSP Bios Loaded\n");

		//Patch Bios: Enable std::io
		uint32_t patch[] = { 0x01, 0x00, 0x01, 0x24, 0xe1, 0x19, 0xf0, 0x0f, 0xc0, 0xa9, 0x81, 0xaf };
		for(int i=0;i<12;i++)
			rom[0x06f0c + i] = patch[i];
		printf("PSP Bios Patched - Enable std_io\n");

		return true;
	}
	printf("PSP Bios Not Found\n");

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
