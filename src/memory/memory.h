#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <memory>

#include "litelib.h"

constexpr auto RAM_SIZE = 0x200000;	//2 MB
constexpr auto CACHE_SIZE = 0x400; //1KB

class Psx;

class Memory
{
public:
	Memory();
	~Memory();

	bool reset();

	uint32_t read(uint32_t phAddr, uint8_t bytes = 4);
	bool write(uint32_t phAddr, uint32_t& data, uint8_t bytes = 4);
	bool writeAddr(uint32_t addr, uint32_t& data, uint8_t bytes = 4);
	uint32_t readAddr(uint32_t addr, uint8_t bytes = 4);

	//Connect to PSX Instance
	void link(Psx* instance) { psx = instance; }

public:
	uint8_t		ram[RAM_SIZE];
	uint8_t		cache[CACHE_SIZE];

private:
	//Link to Bus Object
	Psx* psx;

	//Internal Registers
	uint32_t	ramSize;
};

