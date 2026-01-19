#include <loguru.hpp>
#include "psx.h"

Psx::Psx()
{
	//Reset Master Clock
	masterClock = 0;

	cpu = std::make_shared<CpuShort>();
	//cpu = std::make_shared<CpuFull>();
	gpu = std::make_shared<GPU>();
	spu = std::make_shared<SPU>();
	mem = std::make_shared<Memory>();
	bios = std::make_shared<Bios>();
	dma = std::make_shared<Dma>();
	cdrom = std::make_shared<Cdrom>();
	timers = std::make_shared<Timers>();
	controller = std::make_shared<Controller>();
	tty = std::make_shared<Tty>();
	interrupt = std::make_shared<Interrupt>();
	exeFile = std::make_shared<exefile>();

	//Link Hardware Devices
	cpu->link(this);
	gpu->link(this);
	spu->link(this);
	mem->link(this);
	bios->link(this);
	dma->link(this);
	cdrom->link(this);
	timers->link(this);
	controller->link(this);
	tty->link(this);
	interrupt->link(this);

#ifdef DEBUGGER_ENABLED
    //Init PSX Debugger
    Debugger::link(this);
	Debugger::init();
	cpu->setKernelCallCallback([this](KernelCallEvent e){ Debugger::instance().getCallStackInfo(e); });
#endif

	//Data Bus Status
	dataBusBusy = false;

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

	//Load Bios and Game images
	bios->loadBios(commandline::instance().getBiosFileName());
	cdrom->loadImage(commandline::instance().getBinFileName());
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
	tty->reset();
	interrupt->reset();

	//Data Bus Status
	dataBusBusy = false;

	return true;
}

bool Psx::execute()
{
	//-------------------------------------------------------------------
	// CPU Clock:			33.8688 MHz (Master Clock) [44.1KHz * 600h / 2]
	// TIMER Clock :		33.8688 MHz (CPU Clock) [System8 Clock = 1/8 CPU Clock]
	// GPU Clock:			11/7 CPU Clock
	// CDROM Clock:			1/2 CPU Clock (16.9344 MHz)
	// CONTROLLER Clock:	250.88KHz 1/135 CPU Clock
	// 
	// GPU Clock (PAL):  	53.203425 MHz
	// GPU Clock (NTSC): 	53.693175 MHz
	//-------------------------------------------------------------------

	//Execute all devices
	cpu->execute();
	gpu->runticks();
	dma->execute();
	controller->execute();
	timers->execute(ClockSource::System);

	if (!(masterClock % 2))
	{
		cdrom->execute();
	}

	if (!(masterClock % 8))
	{
		timers->execute(ClockSource::System8);
	}	

	interrupt->execute();

	masterClock++;

	return true;
}

uint32_t Psx::rdMem(uint32_t vAddr, uint8_t bytes)
{
	uint32_t phAddr;
	uint32_t data = 0;
	bool cache;

	readingAddress = vAddr;

	cache = utility::Virtual2PhisicalAddr(vAddr, phAddr);

	//ROM Read Access (BIOS)
	if (memRangeBIOS.contains(phAddr)) return bios->read(phAddr);
	//RAM Read Access
	if (memRangeRAM.contains(phAddr))  return mem->read(phAddr, bytes);

	//Memory Mapped I/O Devices
	if (memRangeGPU.contains(phAddr))  return gpu->readAddr(phAddr, bytes);
	if (memRangeSPU.contains(phAddr))  return spu->readAddr(phAddr, bytes);
	if (memRangeDMA.contains(phAddr))  return dma->readAddr(phAddr, bytes);
	if (memRangeTMR.contains(phAddr))  return timers->readAddr(phAddr, bytes);
	if (memRangeCDR.contains(phAddr))  return cdrom->readAddr(phAddr, bytes);
	if (memRangeINT.contains(phAddr))  return interrupt->readAddr(phAddr, bytes);
	if (memRangeCNT.contains(phAddr))  return controller->readAddr(phAddr, bytes);	
	
	//Debug Bios TTY
	if (memRangeTTY.contains(phAddr))  return tty->readAddr(phAddr, bytes);
		
	LOG_F(ERROR, "Unhandled Memory Read  - addr: 0x%08x (%d)", vAddr, bytes);
	return 0;	
}

bool Psx::wrMem(uint32_t vAddr, uint32_t& data, uint8_t bytes)
{
	uint32_t phAddr;
	bool cache;

	writingAddress = vAddr;

	cache = utility::Virtual2PhisicalAddr(vAddr, phAddr);	
	
	//RAM Write Access
	if (memRangeRAM.contains(phAddr))  return mem->write(phAddr, data, bytes);

	//Memory Mapped I/O Devices
	if (memRangeGPU.contains(phAddr))  return gpu->writeAddr(phAddr, data, bytes);
	if (memRangeSPU.contains(phAddr))  return spu->writeAddr(phAddr, data, bytes);
	if (memRangeDMA.contains(phAddr))  return dma->writeAddr(phAddr, data, bytes);
	if (memRangeTMR.contains(phAddr))  return timers->writeAddr(phAddr, data, bytes);
	if (memRangeCDR.contains(phAddr))  return cdrom->writeAddr(phAddr, data, bytes);
	if (memRangeINT.contains(phAddr))  return interrupt->writeAddr(phAddr, data, bytes);
	if (memRangeCNT.contains(phAddr))  return controller->writeAddr(phAddr, data, bytes);
	
	if (memRangeMEM1.contains(phAddr)) return this->writeAddr(phAddr, data, bytes);
	if (memRangeMEM2.contains(phAddr)) return mem->writeAddr(phAddr, data, bytes);
	if (memRangePOST.contains(phAddr))  return this->writeAddr(phAddr, data, bytes);

	//Debug Bios TTY
	if (memRangeTTY.contains(phAddr))  return tty->writeAddr(phAddr, data, bytes);

	LOG_F(ERROR, "Unhandled Memory Write - addr: 0x%08x, data: 0x%08x (%d)", vAddr, data, bytes);

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
		LOG_F(ERROR, "PSX - Write Unknown Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data); 	
		return false;
	}

	LOG_F(3, "PSX - Write to Register:\t\t0x%08x (%d), data: 0x%08x", addr, bytes, data); 	
	return true;
}
uint32_t Psx::readAddr(uint32_t addr, uint8_t bytes)
{
	uint32_t data = 0x0;

	switch(addr)
	{
	case 0:
		break;

	default:
		data = 0;
		LOG_F(ERROR, "PSX - Read Unknown Register:\t\t0x%08x (%d), data: 0x%08x", addr, bytes, data); 	
		return 0x0;
		break;
	}

	LOG_F(3, "PSX - Read from Register:\t\t0x%08x (%d), data: 0x%08x", addr, bytes, data); 	
	return data;
}