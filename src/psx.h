#pragma once

#include <cstdint>
#include <cstdio>
#include <memory>

#include "litelib.h"

//#include "cpu_full_pipe.h"
#include "cpu_short_pipe.h"
#include "gpu.h"
#include "spu.h"
#include "memory.h"
#include "bios.h"
#include "dma.h"
#include "cdrom.h"
#include "timers.h"
#include "tty.h"
#include "interrupt.h"
#include "controller.h"

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
	bool	 writeAddr(uint32_t addr, uint32_t& data, uint8_t bytes = 4);
	uint32_t readAddr(uint32_t addr, uint8_t bytes = 4);
	
public:
	//PSP Memory Components
	std::shared_ptr<Bios>		bios;
	std::shared_ptr<Memory>		mem;

	//PSP Devices and Peripherals
	std::shared_ptr<CPU>		cpu;
	std::shared_ptr<GPU>		gpu;
	std::shared_ptr<SPU>		spu;
	std::shared_ptr<Dma>		dma;
	std::shared_ptr<Cdrom>		cdrom;
	std::shared_ptr<Timers>		timers;
	std::shared_ptr<Controller> controller;
	std::shared_ptr<Tty> 		tty;
	std::shared_ptr<Interrupt>	interrupt;

	//Mater Clock: 372.5535MHz
	uint64_t	masterClock;

	//Temporary Debug
	uint32_t writingAddress;
	uint32_t readingAddress;

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
	lite::range memRangeRAM =  lite::range(0x00000000, 0x200000);
	lite::range memRangeEXP1 = lite::range(0x1f000000, 0x800000);
	lite::range memRangeExpROM = lite::range(0x1f000000, 0x100);
	lite::range memRangeMEM1 = lite::range(0x1f801000, 0x24);
	lite::range memRangeIOPP = lite::range(0x1f801040, 0x20);
	lite::range memRangeMEM2 = lite::range(0x1f801060, 0x4);
	lite::range memRangeINT = lite::range(0x1f801070, 0x8);
	lite::range memRangeDMA =  lite::range(0x1f801080, 0x80);
	lite::range memRangeTMR =  lite::range(0x1f801100, 0x30);
	lite::range memRangeCDR =  lite::range(0x1f801800, 0x4);
	lite::range memRangeGPU =  lite::range(0x1f801810, 0x8);
	lite::range memRangeSPU =  lite::range(0x1f801c00, 0x400);
	lite::range memRangeEXP2 = lite::range(0x1f802000, 0x1000);
	lite::range memRangeTTY = lite::range(0x1f802020, 0x10);
	lite::range memRangePOST =  lite::range(0x1f802041, 0x1);
	lite::range memRangeEXP3 = lite::range(0x1fa00000, 0x200000);
	lite::range memRangeBIOS = lite::range(0x1fc00000, 0x80000);
	lite::range memRangeCNT = lite::range(0x1f801040, 0x10);
};

