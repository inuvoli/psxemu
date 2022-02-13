#include "psx.h"

Psx::Psx()
{
	//Reset Master Clock
	masterClock = 0;

	cpu = std::make_shared<CPU>();
	gpu = std::make_shared<GPU>();
	spu = std::make_shared<SPU>();
	mem = std::make_shared<Memory>();
	bios = std::make_shared<Bios>();
	dma = std::make_shared<Dma>();
	cdrom = std::make_shared<Cdrom>();
	timers = std::make_shared<Timers>();
	tty = std::make_shared<Tty>();

	//Link Hardware Devices
	cpu->link(this);
	gpu->link(this);
	spu->link(this);
	mem->link(this);
	bios->link(this);
	dma->link(this);
	cdrom->link(this);
	timers->link(this);
	tty->link(this);

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

	cpu->reset();
	gpu->reset();
	dma->reset();
	mem->reset();
	timers->reset();
	cdrom->reset();

	return true;
}

bool Psx::clock()
{
	//-------------------------------------------------------------------
	// GPU Clock:  53.2000 Mhz
	// CPU Clock:  33.8685 MHz
	// CDR Clock: 360.4680 kHz
	//
	// CPU Clock is 7/11 of the GPU Clock. This is achieved starting from
	// a Master Clock at 372.5535 Mhz then:
	// CPU Clock		= Master Clock / 11 (2)
	// GPU Clock		= Master Clock / 7  (1)
	// System/8 Clock	= Master Clock / 88 (16)
	// CDRom Clock 		= Master Clock / 11 * 2352 / 4 / 44100	SystemClock*930h/4/44100Hz
	//-------------------------------------------------------------------

	if (!(masterClock % 2))
	{
		cpu->clock();
		dma->clock();
		timers->clock(ClockSource::System);
	}

	if (!(masterClock % 1))
	{
		gpu->clock();
	}

	if (!(masterClock % 16))
	{
		timers->clock(ClockSource::System8);
	}

	if (!(masterClock % 150))
	{
		cdrom->clock(); //Temporary
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

	readingAddress = vAddr;

	cache = convertVirtualAddr(vAddr, phAddr);

	//ROM Read Access (BIOS)
	if (memRangeBIOS.contains(phAddr)) return bios->read(phAddr);
	//RAM Read Access
	if (memRangeRAM.contains(phAddr))  return mem->read(phAddr, bytes);

	//Memory Mapped I/O Devices
	if (memRangeDMA.contains(phAddr))  return dma->readAddr(phAddr, bytes);
	if (memRangeTMR.contains(phAddr))  return timers->readAddr(phAddr, bytes);
	if (memRangeCDR.contains(phAddr))  return cdrom->readAddr(phAddr, bytes);
	if (memRangeGPU.contains(phAddr))  return gpu->readAddr(phAddr, bytes);
	if (memRangeSPU.contains(phAddr))  return spu->readAddr(phAddr, bytes);
	if (memRangeTTY.contains(phAddr))  return tty->readAddr(phAddr, bytes);	
	
	printf("Unhandled Memory Read  - addr: 0x%08x (%d)\n", vAddr, bytes);

	return 0;	
}

bool Psx::wrMem(uint32_t vAddr, uint32_t& data, uint8_t bytes)
{
	uint32_t phAddr;
	bool cache;

	writingAddress = vAddr;

	cache = convertVirtualAddr(vAddr, phAddr);	
	
	//RAM Write Access
	if (memRangeRAM.contains(phAddr))  return mem->write(phAddr, data, bytes);

	//Memory Mapped I/O Devices
	if (memRangeMEM1.contains(phAddr)) return this->writeAddr(phAddr, data, bytes);
	if (memRangeMEM2.contains(phAddr)) return mem->writeAddr(phAddr, data, bytes);
	if (memRangeDMA.contains(phAddr))  return dma->writeAddr(phAddr, data, bytes);
	if (memRangeTMR.contains(phAddr))  return timers->writeAddr(phAddr, data, bytes);
	if (memRangeCDR.contains(phAddr))  return cdrom->writeAddr(phAddr, data, bytes);
	if (memRangeGPU.contains(phAddr))  return gpu->writeAddr(phAddr, data, bytes);
	if (memRangeSPU.contains(phAddr))  return spu->writeAddr(phAddr, data, bytes);
	if (memRangePOST.contains(phAddr))  return this->writeAddr(phAddr, data, bytes);
	if (memRangeTTY.contains(phAddr))  return tty->writeAddr(phAddr, data, bytes);

	printf("Unhandled Memory Write - addr: 0x%08x, data: 0x%08x (%d)\n", vAddr, data, bytes);

	return false;
}
bool Psx::writeAddr(uint32_t addr, uint32_t& data, uint8_t bytes)
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
uint32_t Psx::readAddr(uint32_t addr, uint8_t bytes)
{
	uint32_t data;

	switch(addr)
	{
	default:
		data = 0;
		printf("PSX - Unknown Parameter Get addr: 0x%08x (%d)\n", addr, bytes);
		break;
	}

	return data;
}