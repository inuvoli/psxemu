#pragma once

#include <cstdint>
#include <cstdio>

#include "litelib.h"


// DMA Channel Registers Fields
union chcrFields
{
	uint32_t		word;

	lite::bitfield<0, 1>	fromRam;		//Transfer Direction (false-> to Ram, true-> from Ram)
	lite::bitfield<1, 1>	memStep;		//Memory Step Increment (0-> +4, 1-> -4)
	lite::bitfield<8, 1>	chopEnable;		//Chopping Enabled
	lite::bitfield<9, 2>	syncMode;		//SyncMode 
	lite::bitfield<16, 3>   chopDmaWnd;		//Chopping DMA Windows
	lite::bitfield<20, 3>	chopCpuWnd;		//Chopping CPU Windows
	lite::bitfield<24, 1>	dmaStart;		//DMA Channel Start/Stop
	lite::bitfield<28, 1>	triggerMode;	//SyncMode 0 Trigger (false->Normal, true->Manual Start)
};

union bcrFields
{
	uint32_t			word;

	lite::bitfield<0, 16>		blockSize;		
	lite::bitfield<16, 16>	blockAmount;
};

class DmaChannel
{
public:
	DmaChannel();
	~DmaChannel();

	bool writeAddr(uint32_t addr, uint32_t& data, uint8_t bytes = 4);
	uint32_t readAddr(uint32_t addr, uint8_t bytes = 4);

public:
	//Channel Registers
	uint32_t	chanMadr;	//DMA Channel Base Address;
	bcrFields	chanBcr;	//DMA Block Control
	chcrFields	chanChcr;	//DMA Channel Control
};

