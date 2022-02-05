#pragma once

#include <string>
#include <fstream>
#include <memory>

constexpr auto BIOS_SIZE = 0x80000;	//512 KB

class Psx;

class Bios
{
public:
	Bios();
	~Bios();

	bool loadBios(const std::string& fileName);
	
	uint32_t rdMem(uint32_t phAddr, uint8_t bytes);
	
	//Connect to PSX Instance
	void link(Psx* instance) { psx = instance; }

	uint8_t		rom[BIOS_SIZE];
	
private:
	//Link to PSP Object
	Psx* psx;
};

