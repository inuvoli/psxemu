#include "psx.h"
#include "common.h"

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

	//Disassemble Bios
	//codeList = cpu.disassemble(0xbfc00000, 0xbfc7ffff);

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
	// CPU Clock		= Master Clock / 11
	// GPU Clock		= Master Clock / 7
	// System/8 Clock	= Master Clock / 88
	//-------------------------------------------------------------------

	if (!(masterClock % 7))
	{
		gpu.clock();
	}

	if (!(masterClock % 11))
	{
		cpu.clock();
		dma.clock();
		timers.clock(ClockSource::System);
	}

	if (!(masterClock % 88))
	{
		timers.clock(ClockSource::System8);
	}
	
	masterClock++;

	return true;
}

uint32_t Psx::rdMem(uint32_t vAddr, uint8_t bytes)
{
	uint32_t maskIndex;
	uint32_t phAddr;
	uint32_t data = 0;
	bool cache;

	//Mask Region MSBs and Check if Region is Cached
	maskIndex = vAddr >> 29;
	phAddr = vAddr & regionMask[maskIndex];
	cache = cacheMask[maskIndex];
	
	//RAM (2MB)
	if (isInRange(phAddr, 0x00000000, 0x00200000))			//RAM (2MB) ----------------------------------------------------------------------------
	{
		return mem.read(phAddr, bytes);
	}
	else if (isInRange(phAddr, 0x1f000000, 0x1f800000))		//Expansion Region 1 (8MB) -------------------------------------------------------------
	{
		printf("Unhandled Read Expansion Region 1 - addr: 0x%08x (%d)\n", vAddr, bytes);
		return 0xffffffff;
	}
	else if (isInRange(phAddr, 0x1f801000, 0x1f802000))		//I/O Ports (4K) -----------------------------------------------------------------------
	{
		if (isInRange(phAddr, 0x1f801080, 0x1f801100))		//DMA Registers
		{
			return dma.getParameter(phAddr, bytes);
		}
		if (isInRange(phAddr, 0x1f801100, 0x1f801130))		//Timers Registers
		{
			return timers.getParameter(phAddr, bytes);
		}
		if (isInRange(phAddr, 0x1f801800, 0x1f801804))		//CDROM Registers
		{
			return cdrom.getParameter(phAddr, bytes);
		}
		if (isInRange(phAddr, 0x1f801810, 0x1f801818))		//GPU Control Registers
		{
			return gpu.getParameter(phAddr, bytes);
		}
		if (isInRange(phAddr, 0x1f801c00, 0x1f802000))		//SPU Control/Voice/Reverb/Internal Registers
		{
			return spu.getParameter(phAddr, bytes);
		}
		
		printf("Unhandled Read I/O Ports - addr: 0x%08x (%d)\n", vAddr, bytes);
	}
	else if (isInRange(phAddr, 0x1f802000, 0x1f803000))		//Expansion Region 2 (4K) ----------------------------------------------------------------
	{
		printf("Unhandled Read Expansion Region 2 - addr: 0x%08x (%d)\n", vAddr, bytes);
		return 0;
	}
	else if (isInRange(phAddr, 0x1fa00000, 0x1fc00000))		//Expansion Region 3 (2MB) ---------------------------------------------------------------
	{
		printf("Unhandled Read Expansion Region 3 - addr: 0x%08x (%d)\n", vAddr, bytes);
		return 0;
	}
	//Bios (512K)
	else if (isInRange(phAddr, 0x1fc00000, 0x1fc80000))		//BIOS (512K) --------------------------------------------------------------------------
	{
		return bios.rdMem(phAddr, 4);
	}
	else
	{
		printf("Unhandled Read - addr: 0x%08x (%d)\n", vAddr, bytes);
		return 0;
	}

	return 0;
}

bool Psx::wrMem(uint32_t vAddr, uint32_t& data, uint8_t bytes)
{
	uint32_t maskIndex;
	uint32_t phAddr;
	bool cache;

	//Mask Region MSBs and Check if Region is Cached
	maskIndex = vAddr >> 29;
	phAddr = vAddr & regionMask[maskIndex];
	cache = cacheMask[maskIndex];
	
	if (isInRange(phAddr, 0x00000000, 0x00200000))			//RAM (2MB) ----------------------------------------------------------------------------
	{
		mem.write(phAddr, data, bytes);
	}
	else if (isInRange(phAddr, 0x1f000000, 0x1f800000))		//Expansion Region 1 (8MB) -------------------------------------------------------------
	{
		printf("Unhandled Write Expansion Region 1 - addr: 0x%08x (%d), data :0x%08x\n", vAddr, bytes, data);
	}
	else if (isInRange(phAddr, 0x1f801000, 0x1f802000))		//I/O Ports (4K) -----------------------------------------------------------------------
	{
		if (isInRange(phAddr, 0x1f801000, 0x1f801024))		//Memory Control 1
		{
			return setParameter(phAddr, data, bytes);
		}
		if (isInRange(phAddr, 0x1f801060, 0x1f801064))		//Memory Control 2
		{
			return mem.setParameter(phAddr, data, bytes);
		}
		if (isInRange(phAddr, 0x1f801080, 0x1f801100))		//DMA Registers
		{
			return dma.setParameter(phAddr, data, bytes);
		}
		if (isInRange(phAddr, 0x1f801100, 0x1f801130))		//Timers Registers
		{
			return timers.setParameter(phAddr, data, bytes);
		}
		if (isInRange(phAddr, 0x1f801800, 0x1f801804))		//CDROM Registers
		{
			return cdrom.setParameter(phAddr, data, bytes);
		}
		if (isInRange(phAddr, 0x1f801810, 0x1f801818))		//GPU Control Registers
		{
			return gpu.setParameter(phAddr, data, bytes);
		}
		if (isInRange(phAddr, 0x1f801c00, 0x1f802000))		//SPU Control Registers
		{
			return spu.setParameter(phAddr, data, bytes);
		}

		printf("Unhandled Write I/O Ports - addr: 0x%08x (%d), data: 0x%08x\n", vAddr, bytes, data);
	}
	else if (isInRange(phAddr, 0x1f802000, 0x1f803000))		//Expansion Region 2 (4K) ----------------------------------------------------------------
	{
		if (isInRange(phAddr, 0x1f802000, 0x1f802080))		//INT/DIP/POST Status
		{
			return setParameter(phAddr, data, bytes);
		}
		printf("Unhandled Write Expansion Region 2 - addr: 0x%08x (%d), data: 0x%08x\n", vAddr, bytes, data);
	}
	else if (isInRange(phAddr, 0x1fa00000, 0x1fc00000))		//Expansion Region 3 (2MB) ---------------------------------------------------------------
	{
		printf("Unhandled Write Expansion Region 3 - addr: 0x%08x (%d), data: 0x%08x\n", vAddr, bytes, data);
	}
	else
	{
		printf("Unhandled Write - addr: 0x%08x (%d), data: 0x%08x\n", vAddr, bytes, data);
	}

	return true;
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