#include "psx.h"

Psx::Psx()
{
	//Reset Master Clock
	masterClock = 0;

	//Link Hardware Devices
	cpu.link(this);
	gpu.link(this);
	spu.link(this);
	mem.link(this);
	bios.link(this);
	dma.link(this);
	cdrom.link(this);
	timers.link(this);

	//Reset Internal Parameters
	exp1BaseAddr = 0x0;
	exp2BaseAddr = 0x0;
	exp1DelaySize = 0x0;
	exp2DelaySize = 0x0;
	exp3DelaySize = 0x0;
	biosDelaySize = 0x0;
	spuDelaySize = 0x0;
	cdromDelaySize = 0x0;
	comDelay = 0x0;
	postStatus = 0x0;
}

Psx::~Psx()
{
}

bool Psx::reset()
{
	//Reset Master Clock
	masterClock = 0;

	//Reset Internal Parameters
	exp1BaseAddr = 0x0;
	exp2BaseAddr = 0x0;
	exp1DelaySize = 0x0;
	exp2DelaySize = 0x0;
	exp3DelaySize = 0x0;
	biosDelaySize = 0x0;
	spuDelaySize = 0x0;
	cdromDelaySize = 0x0;
	comDelay = 0x0;
	postStatus = 0x0;

	cpu.reset();
	gpu.reset();
	dma.reset();
	mem.reset();
	timers.reset();

	return true;
}

bool Psx::clock()
{
	//-------------------------------------------------------------------
	// GPU Clock: 53.2000 Mhz
	// CPU Clock: 33.8685 MHz
	//
	// CPU Clock is 7/11 of the GPU Clock. This is achieved starting from
	// a Master Clock at 372.5535 Mhz then:
	// CPU Clock		= Master Clock / 11 (2)
	// GPU Clock		= Master Clock / 7  (1)
	// System/8 Clock	= Master Clock / 88 (16)
	//-------------------------------------------------------------------

	if (!(masterClock % 2))
	{
		cpu.clock();
		dma.clock();
		timers.clock(ClockSource::System);
	}

	if (!(masterClock % 1))
	{
		gpu.clock();
	}

	if (!(masterClock % 16))
	{
		timers.clock(ClockSource::System8);
	}

	masterClock++;

	return true;
}

bool Psx::convertVirtualAddr(uint32_t vAddr, uint32_t& phAddr)
{
	uint32_t maskIndex;

	//Mask Region MSBs and Check if Region is Cached
	maskIndex = vAddr >> 29;
	phAddr = vAddr & regionMask[maskIndex];
	return cacheMask[maskIndex];
}

uint32_t Psx::rdMem(uint32_t vAddr, uint8_t bytes)
{
	uint32_t phAddr;
	uint32_t data = 0;
	bool cache;

	cache = convertVirtualAddr(vAddr, phAddr);

	if (memRangeRAM.contains(phAddr))  return mem.read(phAddr, bytes);
	if (memRangeDMA.contains(phAddr))  return dma.getParameter(phAddr, bytes);
	if (memRangeTMR.contains(phAddr))  return timers.getParameter(phAddr, bytes);
	if (memRangeCDR.contains(phAddr))  return cdrom.getParameter(phAddr, bytes);
	if (memRangeGPU.contains(phAddr))  return gpu.getParameter(phAddr, bytes);
	if (memRangeSPU.contains(phAddr))  return spu.getParameter(phAddr, bytes);
	if (memRangeBIOS.contains(phAddr)) return bios.rdMem(phAddr, 4);
	
	printf("Unhandled Memory Read  - addr: 0x%08x (%d)\n", vAddr, bytes);

	return 0;	
}

bool Psx::wrMem(uint32_t vAddr, uint32_t& data, uint8_t bytes)
{
	uint32_t phAddr;
	bool cache;

	cache = convertVirtualAddr(vAddr, phAddr);	
	
	if (memRangeRAM.contains(phAddr))  return mem.write(phAddr, data, bytes);
	if (memRangeMEM1.contains(phAddr)) return setParameter(phAddr, data, bytes);
	if (memRangeMEM2.contains(phAddr)) return mem.setParameter(phAddr, data, bytes);
	if (memRangeDMA.contains(phAddr))  return dma.setParameter(phAddr, data, bytes);
	if (memRangeTMR.contains(phAddr))  return timers.setParameter(phAddr, data, bytes);
	if (memRangeCDR.contains(phAddr))  return cdrom.setParameter(phAddr, data, bytes);
	if (memRangeGPU.contains(phAddr))  return gpu.setParameter(phAddr, data, bytes);
	if (memRangeSPU.contains(phAddr))  return spu.setParameter(phAddr, data, bytes);
	if (memRangeIDP.contains(phAddr))  return setParameter(phAddr, data, bytes);

	printf("Unhandled Memory Write - addr: 0x%08x, data: 0x%08x (%d)\n", vAddr, data, bytes);

	return false;
}
bool Psx::setParameter(uint32_t addr, uint32_t& data, uint8_t bytes)
{
	switch (addr)
	{
	case 0x1f801000:		//Expansion 1 Base Address
		exp1BaseAddr = data;
		break;
	case 0x1f801004:		//Expansion 2 Base Address
		exp2BaseAddr = data;
		break;
	case 0x1f801008:		//Expansion 1 Delay/Size
		exp1DelaySize = data;
		break;
	case 0x1f80100c:		//Expansion 3 Delay/Size
		exp3DelaySize = data;
		break;
	case 0x1f801010:		//Bios Rom Delay/Size
		biosDelaySize = data;
		break;
	case 0x1f801014:		//SPU Delay/Size
		spuDelaySize = data;
		break;
	case 0x1f801018:		//CDROM Delay/Size
		cdromDelaySize = data;
		break;
	case 0x1f80101c:		//Expansion 2 Delay/Size
		exp2DelaySize = data;
		break;				//Common Delay
	case 0x1f801020:
		comDelay = data;
		break;
	case 0x1f802041:		//POST Status
		postStatus = data;
		break;
	default:
		printf("PSX - Unknown Parameter Set addr: 0x%08x (%d), data: 0x%08x\n", addr, bytes, data);
		return false;
	}
	return true;
}
uint32_t Psx::getParameter(uint32_t addr, uint8_t bytes)
{
	printf("PSX - Unknown Parameter Set addr: 0x%08x (%d)\n", addr, bytes);
	return 0;
}