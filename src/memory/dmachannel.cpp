#include "dmachannel.h"

DmaChannel::DmaChannel()
{
	//Reset Channel Registers
	chanMadr = 0x00000000;
	chanChcr.word = 0x00000000;
	chanBcr.word = 0x00000000;

}

DmaChannel::~DmaChannel()
{
}

bool DmaChannel::writeAddr(uint32_t addr, uint32_t& data, uint8_t bytes)
{
	//The value of addr identifies the channel according to this association:
	//1F80108xh DMA0 channel 0  MDECin(RAM to MDEC)
	//1F80109xh DMA1 channel 1  MDECout(MDEC to RAM)
	//1F8010Axh DMA2 channel 2  GPU(lists + image data)
	//1F8010Bxh DMA3 channel 3  CDROM(CDROM to RAM)
	//1F8010Cxh DMA4 channel 4  SPU
	//1F8010Dxh DMA5 channel 5  PIO(Expansion Port)
	//1F8010Exh DMA6 channel 6  OTC(reverse clear OT) (GPU related)
	//
	// Every Channel has 3 registers
	// x = 0: DMA Base Address
	// x = 4: DMA Block Control
	// x = 8: DMA Channel Control
	//
	// The value of x is derived from the last 4 bit of addr

	int reg = (addr & 0x0000000f);

	switch (reg)
	{
	case 0x0:
		chanMadr = data & 0x00ffffff; //bit 31-24 of MADR are always zero
		break;
	case 0x4:
		chanBcr.word = data;
		break;
	case 0x8:
		chanChcr.word = data;
		break;

	default:
		printf("DMA Channel Register Set Unknown!\n");
		break;
	}

	return true;
}

uint32_t DmaChannel::readAddr(uint32_t addr, uint8_t bytes)
{
	//The value of addr identifies the channel according to this association:
	//1F80108xh DMA0 channel 0  MDECin(RAM to MDEC)
	//1F80109xh DMA1 channel 1  MDECout(MDEC to RAM)
	//1F8010Axh DMA2 channel 2  GPU(lists + image data)
	//1F8010Bxh DMA3 channel 3  CDROM(CDROM to RAM)
	//1F8010Cxh DMA4 channel 4  SPU
	//1F8010Dxh DMA5 channel 5  PIO(Expansion Port)
	//1F8010Exh DMA6 channel 6  OTC(reverse clear OT) (GPU related)
	//
	// Every Channel has 3 registers
	// x = 0: DMA Base Address
	// x = 4: DMA Block Control
	// x = 8: DMA Channel Control
	//
	// The value of x is derived from the last 4 bit of addr

	int data = 0;
	int reg = (addr & 0x0000000f);

	switch (reg)
	{
	case 0x0:
		data = chanMadr;
		break;
	case 0x4:
		data = chanBcr.word;
		break;
	case 0x8:
		data = chanChcr.word;
		break;

	default:
		printf("DMA Channel Register Get Unknown!\n");
		break;
	}

	return data;
}
