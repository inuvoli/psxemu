#pragma once

#include <cstdint>
#include <cstdio>

//#include "cpu_full_pipe.h"
#include "cpu_short_pipe.h"
#include "gpu.h"
#include "spu.h"
#include "memory.h"
#include "bios.h"
#include "dma.h"
#include "cdrom.h"
#include "timers.h"
#include "range.h"

class Psx
{
public:
	Psx();
	~Psx();
		
	bool reset();
	bool clock();

	//Memory Bus Access
	uint32_t rdMem(uint32_t vAddr, uint8_t bytes = 4);
	bool	 wrMem(uint32_t vAddr, uint32_t& data, uint8_t bytes = 4);
	bool	 setParameter(uint32_t addr, uint32_t& data, uint8_t bytes = 4);
	uint32_t getParameter(uint32_t addr, uint8_t bytes = 4);


	
public:
	//PSP Hardware Components
	CPU				cpu;
	GPU				gpu;
	SPU				spu;
	Memory			mem;
	Bios			bios;
	Dma				dma;
	Cdrom			cdrom;
	Timers			timers;

	//Mater Clock: 372.5535MHz
	uint64_t	masterClock;

	//Internal Registers
	uint32_t exp1BaseAddr;		//0x1f801000
	uint32_t exp2BaseAddr;		//0x1f801004
	uint32_t exp1DelaySize;		//0x1f801008
	uint32_t exp2DelaySize;		//0x1f80101c
	uint32_t exp3DelaySize;		//0x1f80100c
	uint32_t biosDelaySize;		//0x1f801010
	uint32_t spuDelaySize;		//0x1f801014
	uint32_t cdromDelaySize;	//0x1f801018
	uint32_t comDelay;			//0x1f801020
	uint32_t postStatus;		//0x1f802041

private:
	bool convertVirtualAddr(uint32_t vAddr, uint32_t& phAddr);

private:
	//Memory Conversion Helper
	static constexpr uint32_t	regionMask[8] = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x7fffffff, 0x1fffffff, 0xffffffff, 0xffffffff };
	static constexpr bool		cacheMask[8] = { true, true, true, true, true, false, true, true };

	//Memory Mapping
	Range memRangeRAM =  Range(0x00000000, 0x200000);
	Range memRangeEXP1 = Range(0x1f000000, 0x800000);
	Range memRangeMEM1 = Range(0x1f801000, 0x24);
	Range memRangeIOPP = Range(0x1f801040, 0x20);
	Range memRangeMEM2 = Range(0x1f801060, 0x4);
	Range memRangeDMA =  Range(0x1f801080, 0x80);
	Range memRangeTMR =  Range(0x1f801100, 0x30);
	Range memRangeCDR =  Range(0x1f801800, 0x4);
	Range memRangeGPU =  Range(0x1f801810, 0x8);
	Range memRangeSPU =  Range(0x1f801c00, 0x400);
	Range memRangeEXP2 = Range(0x1f802000, 0x1000);
	Range memRangeIDP =  Range(0x1f802000, 0x80);
	Range memRangeEXP3 = Range(0x1fa00000, 0x200000);
	Range memRangeBIOS = Range(0x1fc00000, 0x80000);
};

