#pragma once

#include <cstdint>
#include <cstdio>
#include <string>
#include <array>
#include <algorithm>
#include <memory>

#include "dmachannel.h"
#include "range.h"

//DMA Constant Definitions
constexpr auto DMA_CHANNEL_NUMBER	= 7;				//4 KB

// DMA Channel Registers Fields
union dicrFields
{
	uint32_t		word;

	BitField<15, 1>	forceIrq;			//(0=None, 1=Force Bit31=1)
	BitField<16, 7>	enableIrq;			//DMA0-DMA6 IRQ Enable (0=None, 1=Enable)
	BitField<23, 1>	masterEnableIrq;	//Master IRQ Enable (0=None, 1=Enable)
	BitField<24, 7>	flagsIrq;			//DMA0-DMA6 IRQ Flags (0=None, 1=IRQ)    (Write 1 to reset)
	BitField<31, 1> masterFlagIrq;		//Chopping DMA Windows
};

class Psx;

struct DmaStatusItem
{
	bool		enabled = false;
	uint8_t		priority = 1;
	uint8_t		channel;
};

class Dma
{
public:
	Dma();
	~Dma();

	bool reset();
	bool clock();

	bool writeAddr(uint32_t addr, uint32_t& data, uint8_t bytes);
	uint32_t readAddr(uint32_t addr, uint8_t bytes);
	
	//Connect to PSX Instance
	void link(Psx* instance) { psx = instance; }

public:
	//DMA Internal Registers
	std::array<DmaStatusItem, DMA_CHANNEL_NUMBER>	dmaStatus;
	DmaChannel					dmaChannel[DMA_CHANNEL_NUMBER]; //DMA Channels
	uint32_t					dmaDpcr;
	dicrFields					dmaDicr;
	
private:
	bool syncmode0();
	bool syncmode1();
	bool syncmode2();
	bool updateDicr(uint32_t data);
	bool interruptCheck();
	bool dmaStop();

private:
	//Link to PSP Object
	Psx* psx;

	bool		isRunning;
	uint8_t		runningChannel;
	uint32_t	runningAddr;
	int8_t		runningIncrement;
	bool		runningFromRam;
	uint32_t	runningSize;
	uint16_t	runningBlockAmount;
	uint16_t	runningBlockSize;
	uint8_t		runningSyncMode;

	//Memory Mapping
	lite::range memRangeChannelRegs =  lite::range(0x1f801080, 0x70);
};

