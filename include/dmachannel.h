#pragma once

#include <cstdint>
#include <cstdio>

#include "bitfield.h"


// DMA Channel Registers Fields
union chcrFields
{
	uint32_t		word;

	BitField<0, 1>	fromRam;		//Transfer Direction (false-> to Ram, true-> from Ram)
	BitField<1, 1>	memStep;		//Memory Step Increment (0-> +4, 1-> -4)
	BitField<8, 1>	chopEnable;		//Chopping Enabled
	BitField<9, 2>	syncMode;		//SyncMode 
	BitField<16, 3> chopDmaWnd;		//Chopping DMA Windows
	BitField<20, 3>	chopCpuWnd;		//Chopping CPU Windows
	BitField<24, 1>	dmaStart;		//DMA Channel Start/Stop
	BitField<28, 1>	triggerMode;	//SyncMode 0 Trigger (false->Normal, true->Manual Start)
};

union bcrFields
{
	uint32_t			word;

	BitField<0, 16>		blockSize;		
	BitField<16, 16>	blockAmount;
};

class DmaChannel
{
public:
	DmaChannel();
	~DmaChannel();

	bool setParameter(uint32_t addr, uint32_t& data, uint8_t bytes = 4);
	uint32_t getParameter(uint32_t addr, uint8_t bytes = 4);

public:
	//Channel Registers
	uint32_t	chanMadr;	//DMA Channel Base Address;
	bcrFields	chanBcr;	//DMA Block Control
	chcrFields	chanChcr;	//DMA Channel Control
};

