#include <loguru.hpp>
#include "dma.h"
#include "psx.h"

Dma::Dma()
{
	//Reset DMA Registers
	memset(dmaChannel, 0x00000000, sizeof(dmaChannel));
	dmaDpcr = 0x07654321;
	dmaDicr.word = 0x00000000;

	for (int i = 0; i < DMA_CHANNEL_NUMBER; i++)
	{
		dmaStatus[i].channel = i;
		dmaStatus[i].priority = i + 1;
		dmaStatus[i].enabled = false;
	}

	isRunning = false;
	runningChannel = 0xff;
	runningSyncMode = 0xff;
	runningAddr = 0x00000000;
	runningIncrement = 0;
	runningFromRam = false;
	runningSize = 0;
	runningBlockAmount = 0;
	runningBlockSize = 0;
}

Dma::~Dma()
{
}

bool Dma::reset()
{
	//Reset DMA Registers
	memset(dmaChannel, 0x00000000, sizeof(dmaChannel));
	dmaDpcr = 0x07654321;
	dmaDicr.word = 0x00000000;

	for (int i = 0; i < DMA_CHANNEL_NUMBER; i++)
	{
		dmaStatus[i].channel = i;
		dmaStatus[i].priority = i + 1;
		dmaStatus[i].enabled = false;
	}

	isRunning = false;
	runningChannel = 0xff;
	runningSyncMode = 0xff;
	runningAddr = 0x00000000;
	runningIncrement = 0;
	runningFromRam = false;
	runningSize = 0;
	runningBlockAmount = 0;
	runningBlockSize = 0;

	return true;
}

bool Dma::clock()
{
	//Check if DMA is already Running
	if (isRunning)
	{
		switch (runningSyncMode)
		{
		case 0:
			syncmode0();		//SyncMode 0 - Immediate Block Transfer
			break;
		case 1:
			syncmode1();		//SyncMode 1 - Triggered Block Transfer
			break;
		case 2:
			syncmode2();		//SyncMode 2 - Linked List
			break;

		default:
			LOG_F(ERROR, "DMA - Channel %d Unknown Sync Mode!", runningChannel);
		}

		return true;
	}

	//Check for any Active DMA Channel
	for (auto& e : dmaStatus)
	{
		if (e.enabled && (bool)dmaChannel[e.channel].chanChcr.dmaStart)
		{
			isRunning = true;
			runningChannel = e.channel;
			runningSyncMode = (uint8_t)dmaChannel[e.channel].chanChcr.syncMode;

			runningAddr = dmaChannel[e.channel].chanMadr;
			runningIncrement = ((bool)dmaChannel[e.channel].chanChcr.memStep) ? -4 : +4;
			runningFromRam = (bool)dmaChannel[e.channel].chanChcr.fromRam;

			switch (runningSyncMode)
			{
			case 0:
				runningSize = dmaChannel[e.channel].chanBcr.blockSize;
				runningBlockAmount = dmaChannel[e.channel].chanBcr.blockAmount;
				runningBlockSize = dmaChannel[e.channel].chanBcr.blockSize;
				break;
			case 1:
				runningSize = dmaChannel[e.channel].chanBcr.blockSize * dmaChannel[e.channel].chanBcr.blockAmount;
				runningBlockAmount = dmaChannel[e.channel].chanBcr.blockAmount;
				runningBlockSize = dmaChannel[e.channel].chanBcr.blockSize;
				break;
			case 2:
				runningSize = 0;
				runningBlockAmount = dmaChannel[e.channel].chanBcr.blockAmount;
				runningBlockSize = dmaChannel[e.channel].chanBcr.blockSize;
				break;

			default:
				break;
			}
			LOG_F(1, "DMA - Active Request: Channel %d, SyncMode %d, BlockSize %d, BlockAmount %d, TotalSize %d, MemoryAddr %08x, Increment %d, FromMemory %d", runningChannel, runningSyncMode, runningBlockSize, runningBlockAmount, runningSize, runningAddr, runningIncrement, runningFromRam);
			
			//Stop CPU access to Address Bus
			psx->cpu->dmaTakeOnBus = true;
		}	
	}
	return true;
}

bool Dma::writeAddr(uint32_t addr, uint32_t& data, uint8_t bytes)
{
	int chn = 0;

	//Setting DMA Channel Registers
	if (memRangeChannelRegs.contains(addr))
	{
		chn = (addr - 0x1f801080) >> 4;
		dmaChannel[chn].writeAddr(addr, data);
		return true;
	}

	//Setting DPCR and DICR Registers
	switch (addr)
	{
	case 0x1f8010f0:
		dmaDpcr = data;

		//Update Priority Order every time DPCR is Updated
		chn = 0;
		for (auto &e : dmaStatus)
		{
			e.enabled = ((data >> (4 * chn)) & 0x00000008) ? true : false;
			e.priority = ((data >> (4 * chn)) & 0x00000007);
			e.channel = chn;
			chn++;
		}
		std::sort(dmaStatus.begin(), dmaStatus.end(), [](dma::StatusItem a, dma::StatusItem b) { return ((a.priority >= b.priority) && (a.channel > b.channel)); });
		break;

	case 0x1f8010f4:
		updateDicr(data);
		break;

	default:
		LOG_F(ERROR, "DMA - Unknown Parameter Set addr: 0x%08x (%d), data: 0x%08x", addr, bytes, data);
		return false;
	}

	return true;
}

uint32_t Dma::readAddr(uint32_t addr, uint8_t bytes)
{
	uint32_t data = 0;

	//Retrieve DMA Channel Registers
	if (memRangeChannelRegs.contains(addr))
	{
		int chn = (addr - 0x1f801080) >> 4;
		data = dmaChannel[chn].readAddr(addr);
		return data;
	}

	//Retrieve DPCR and DICR Registers
	switch (addr)
	{
	case 0x1f8010f0:
		data = dmaDpcr;
		break;
	case 0x1f8010f4:
		data = dmaDicr.word;
		break;

	default:
		LOG_F(ERROR, "DMA - Unknown Parameter Get addr: 0x%08x (%d)", addr, bytes);
		return 0x0;
	}
	
	return data;
}

bool Dma::syncmode0()
{
	uint32_t data;

	if (runningFromRam)
	{
		//TODO
		//Sync Mode 0 should not support reading From RAM
		LOG_F(ERROR, "DMA - Channel %d - Sync Mode 0 read from Ram not supported!", runningChannel);
	}
	else
	{
		switch (runningChannel)
		{
		case 3: //CDROM
			//Read a word from CDROM Data Buffer 1 byte at a time
			data = psx->cdrom->readAddr(0x1f801802, 1);
			data += (psx->cdrom->readAddr(0x1f801802, 1) << 8);
			data += (psx->cdrom->readAddr(0x1f801802, 1) << 16);
			data += (psx->cdrom->readAddr(0x1f801802, 1) << 24);
			break;
		case 6:	//OTC
			data = (runningSize == 1) ? 0x00ffffff : (runningAddr + runningIncrement) & 0x001ffffc;
			break;

		default:
			data = 0;
			LOG_F(ERROR, "DMA - Channel %d not supported in Sync Mode 0", runningChannel);
		}

		//Write data to RAM
		psx->mem->write(runningAddr, data);  //Write data directly to RAM
		runningAddr = (runningAddr + runningIncrement) & 0x001ffffc;
		runningSize--;

		//Check if all data have been written
		if (runningSize == 0)
			dmaStop();
	}
	
	return true;
}

bool Dma::syncmode1()
{
	uint32_t data;

	if (runningFromRam)
	{
		//Read data from current Address
		data = psx->mem->read(runningAddr);

		switch (runningChannel)
		{
		case 2:	//------------------------------------------GPU
			psx->gpu->writeAddr(0x1f801810, data); //Write to GP0
			break;

		default:
			LOG_F(ERROR, "DMA - Channel %d not supported in Sync Mode 0", runningChannel);
		}

		//Update Current Address and DMA Channel Registers
		runningAddr += runningIncrement;
		
		dmaChannel[runningChannel].chanMadr = runningAddr;

		if ((runningSize % dmaChannel[runningChannel].chanBcr.blockSize) == 0)
		{
			dmaChannel[runningChannel].chanBcr.blockAmount--;
		}

		runningSize--;
	}
	else
	{
		//TODO - to Ram, don't know if there's any case
		LOG_F(ERROR, "DMA - Channel %d - Sync Mode 1 write to Ram not supported!", runningChannel);
	}

	if (runningSize == 0)
		dmaStop();

	return true;
}

bool Dma::syncmode2()
{
	uint32_t data;
	
	//Check if we are finished to send packet and need to read the next header
	if (runningSize == 0)
	{
		//Check if we are at the end of the linked list
		if (dmaChannel[runningChannel].chanMadr == 0x00ffffff)
		{
			dmaStop();
			return true;
		}

		//Read Linked List Header content and prepare to send packets
		data = psx->mem->read(dmaChannel[runningChannel].chanMadr);	//Read Header directly from memory
		runningSize = data >> 24;									//Contains the size of the packets expressed in words
		runningAddr = dmaChannel[runningChannel].chanMadr + 4;		//Contains address of the first word of the packet
		dmaChannel[runningChannel].chanMadr = data & 0x00ffffff;	//Update Channel Based Address to point the next Header
	}
	else
	{
		if (runningFromRam)
		{
			switch (runningChannel)
			{
			case 2:	//------------------------------------------GPU
				data = psx->mem->read(runningAddr);
				psx->gpu->writeAddr(0x1f801810, data); //Write to GP0
				runningAddr += 4;
				runningSize--;
				break;

			default:
				LOG_F(ERROR, "DMA - Channel %d - Sync Mode 2 write From Ram not supported!", runningChannel);
				break;
			}
		}
		else
		{
			//TODO - I dont't know if we ever need to receive packet to Ram.
			LOG_F(ERROR, "DMA - Channel %d - Sync Mode 2 write to Ram not supported!", runningChannel);
		}
	}

	return true;
}

inline bool Dma::updateDicr(uint32_t data)
{
	dma::dicr tmp;

	tmp.word = data;
	dmaDicr.forceIrq = tmp.forceIrq;
	dmaDicr.enableIrq = tmp.enableIrq;
	dmaDicr.masterEnableIrq = tmp.masterEnableIrq;
	dmaDicr.flagsIrq = dmaDicr.flagsIrq & ~tmp.flagsIrq;
	dmaDicr.masterFlagIrq = ((bool)dmaDicr.forceIrq || ((bool)dmaDicr.masterEnableIrq && (dmaDicr.enableIrq && dmaDicr.flagsIrq))) ? 1 : 0;
	
	return true;
}

bool Dma::dmaStop()
{
	dmaChannel[runningChannel].chanChcr.dmaStart = 0;
	dmaChannel[runningChannel].chanChcr.triggerMode = 0;

	isRunning = false;
	runningSize = 0;
	runningAddr = 0x00000000;

	psx->cpu->dmaTakeOnBus = false;

	//Set DMA Channel Completion Interrupt Flag, Update DICR and Activate Interrupt if DICR.31 goes from 0 to 1
	if (dmaDicr.enableIrq & (1UL << runningChannel))
	{
		dmaDicr.flagsIrq = dmaDicr.flagsIrq | (1UL << runningChannel);
		dmaDicr.masterFlagIrq = ((bool)dmaDicr.forceIrq || ((bool)dmaDicr.masterEnableIrq && (dmaDicr.enableIrq && dmaDicr.flagsIrq))) ? 1 : 0;

		if (dmaDicr.masterFlagIrq)
			psx->interrupt->set(static_cast<uint32_t>(interruptCause::dma));
	}

	return true;
}
