#include "cdrom.h"
#include "psx.h"

Cdrom::Cdrom()
{
	statusCode.byte = 0;
	requestRegister.byte = 0;
	interruptEnableRegister = 0;
	interruptFlagRegister = 0;
	commandRegister = 0;

	adpcmFifo.flush();
	parameterFifo.flush();
	dataFifo.flush();
	responseFifo.flush();
	interruptFifo.flush();

	//Init StatusRegister
	statusRegister.index = 0;
	statusRegister.adpbusy = 0;
	statusRegister.prmempt = 1;
	statusRegister.prmwrdy = 1;
	statusRegister.rslrrdy = 0;
	statusRegister.drqsts = 0;
	statusRegister.busysts = 0;

	//Init Internal Status
	commandAvailable = false;

	//Init COMMAND Dictionary
	commandSet = 
	{
		{"Unused", &Cdrom::cmd_unused},
		{"Getstat", &Cdrom::cmd_getstat},
		{"Setloc", &Cdrom::cmd_setloc},
		{"Play", &Cdrom::cmd_play},
		{"Forward", &Cdrom::cmd_forward},
		{"Backward", &Cdrom::cmd_backward},
		{"ReanN", &Cdrom::cmd_readn},
		{"MotorOn", &Cdrom::cmd_motoron},
		{"Stop", &Cdrom::cmd_stop},
		{"Pause", &Cdrom::cmd_pause},
		{"Init", &Cdrom::cmd_init},
		{"Mute", &Cdrom::cmd_mute},
		{"Demute", &Cdrom::cmd_demute},
		{"Setfilter", &Cdrom::cmd_setfilter},
		{"Setmode", &Cdrom::cmd_setmode},
		{"Getparam", &Cdrom::cmd_getparam},
		{"GetlocL", &Cdrom::cmd_getlocl},
		{"GetlocP", &Cdrom::cmd_getlocp},
		{"SetSession", &Cdrom::cmd_setsession},
		{"GetTN", &Cdrom::cmd_gettn},
		{"GetTD", &Cdrom::cmd_cettd},
		{"SeekL", &Cdrom::cmd_seekl},
		{"SeekP", &Cdrom::cmd_seekp},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Test", &Cdrom::cmd_test},
		{"GetID", &Cdrom::cmd_getid},
		{"ReadS", &Cdrom::cmd_reads},
		{"Reset", &Cdrom::cmd_reset},
		{"GetQ", &Cdrom::cmd_getq},
		{"ReadTOC", &Cdrom::cmd_readtoc},
		{"VideoCD", &Cdrom::cmd_videocd},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Secret 1", &Cdrom::cmd_secret},
		{"Secret 2", &Cdrom::cmd_secret},
		{"Secret 3", &Cdrom::cmd_secret},
		{"Secret 4", &Cdrom::cmd_secret},
		{"Secret 5", &Cdrom::cmd_secret},
		{"Secret 6", &Cdrom::cmd_secret},
		{"Secret 7", &Cdrom::cmd_secret},
		{"Secret Lock", &Cdrom::cmd_secretlock},
		{"Crash", &Cdrom::cmd_crash},
		{"Crash", &Cdrom::cmd_crash},
		{"Crash", &Cdrom::cmd_crash},
		{"Crash", &Cdrom::cmd_crash},
		{"Crash", &Cdrom::cmd_crash},
		{"Crash", &Cdrom::cmd_crash},
		{"Crash", &Cdrom::cmd_crash},
		{"Crash", &Cdrom::cmd_crash},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused},
		{"Unused", &Cdrom::cmd_unused}
	};
}

Cdrom::~Cdrom()
{
}

bool Cdrom::reset()
{
	statusCode.byte = 0;
	requestRegister.byte = 0;
	interruptEnableRegister = 0;
	interruptFlagRegister = 0;
	commandRegister = 0;

	adpcmFifo.flush();
	parameterFifo.flush();
	dataFifo.flush();
	responseFifo.flush();
	interruptFifo.flush();

	//Init StatusRegister
	statusRegister.index = 0;
	statusRegister.adpbusy = 0;
	statusRegister.prmempt = 1;
	statusRegister.prmwrdy = 1;
	statusRegister.rslrrdy = 0;
	statusRegister.drqsts = 0;
	statusRegister.busysts = 0;

	//Init Internal Status
	commandAvailable = false;
	
	return true;
}

bool Cdrom::clock()
{
	bool bResult;
	uint8_t interruptNum;

	//Update Status Register
	statusRegister.adpbusy = (adpcmFifo.isempty()) ? 0 : 1;
	statusRegister.prmempt = (parameterFifo.isempty()) ? 1 : 0;
	statusRegister.prmwrdy = (parameterFifo.isfull()) ? 0 : 1;
	statusRegister.rslrrdy = (responseFifo.isempty()) ? 0 : 1;
	statusRegister.drqsts = (dataFifo.isempty()) ? 0 : 1;

	//Check for pending Interrupts
	if (!interruptFifo.isempty())
	{
		//Check for other Interrupt still running
		if (!(interruptFlagRegister & 0x7))
		{
			interruptFifo.pop(interruptNum);
			//Check if Interupt is enabled
			if ((interruptEnableRegister & interruptNum) == interruptNum)
			{
				interruptFlagRegister = (interruptFlagRegister & 0xf8) | interruptNum;
				psx->cpu->interrupt(static_cast<uint32_t>(cpu::interruptCause::cdrom));
				//printf("CDROM - INT%d\n", interruptFlagRegister & 0x7);
			}
		}
	}

	//Check for available commands
	if (commandAvailable)
	{
		commandAvailable = false;
		statusRegister.busysts = 0;

		//printf("CDROM - Command %s!\n", commandSet[commandRegister].mnemonic.c_str());
		bResult = (this->*commandSet[commandRegister].operate)();
			if (!bResult)
				printf("CDROM - Unimplemented Command %s!\n", commandSet[commandRegister].mnemonic.c_str());
	}

	return false;
}

bool Cdrom::writeAddr(uint32_t addr, uint32_t& data, uint8_t bytes)
{
	switch (addr)
	{
	case 0x1f801800:
		//printf("CDROM - Write Status Register:\t\t0x%08x       , data: 0x%08x\n", addr, data);
		statusRegister.index = data & 0x3;
		break;
	
	case 0x1f801801:
		switch(statusRegister.index)
		{
		case 0:
			commandRegister = data;
			commandAvailable = true;
			statusRegister.busysts = 1;
			//printf("CDROM - Write Command Register:\t\t0x%08x.Index%d, data: 0x%08x\n", addr, statusRegister.index, data);
			break;

		case 1:
			//Sound Map Data Out
			//printf("CDROM - Write Sound Map Data Register:\t\t0x%08x.Index%d, data: 0x%08x\n", addr, statusRegister.index, data);
			break;

		case 2:
			//Sound Map Coding Info
			//printf("CDROM - Write Sound Map Coding Register:\t\t0x%08x.Index%d, data: 0x%08x\n", addr, statusRegister.index, data);
			break;

		case 3:
			//Audio Volume for Right-CD-Out to Right-SPU-Input
			//printf("CDROM - Write Audio Volume Register:\t\t0x%08x.Index%d, data: 0x%08x\n", addr, statusRegister.index, data);
			break;
		};
		break;

	case 0x1f801802:
		switch(statusRegister.index)
		{
		case 0:
			parameterFifo.push(data);
			//printf("CDROM - Write Parameter Fifo:\t\t0x%08x.Index%d, data: 0x%08x\n", addr, statusRegister.index, data);
			break;

		case 1:
			interruptEnableRegister = data & 0x1f;
			//printf("CDROM - Write Int. Enable Register:\t0x%08x.Index%d, data: 0x%08x\n", addr, statusRegister.index, data);
			break;

		case 2:
			//Audio Volume for Left-CD-Out to Left-SPU-Input
			//printf("CDROM - Write Audio Volume Register:\t\t0x%08x.Index%d, data: 0x%08x\n", addr, statusRegister.index, data); 
			break;

		case 3:
			//Audio Volume for Right-CD-Out to Left-SPU-Input
			//printf("CDROM - Write Audio Volume Register:\t\t0x%08x.Index%d, data: 0x%08x\n", addr, statusRegister.index, data);
			break;
		};
		break;

		case 0x1f801803:
		switch(statusRegister.index)
		{
		case 0:
			requestRegister.byte = data & 0xe0;
			//printf("CDROM - Write Request Register:\t\t0x%08x.Index%d, data: 0x%08x\n", addr, statusRegister.index, data);
			break;

		case 1:
			interruptFlagRegister = (data & 0xe0) | ((interruptFlagRegister & 0x1f) & ~(data & 0x1f));
			if (data & 0x40)
				parameterFifo.flush();

			//printf("CDROM - Write Int. Flag Register:\t0x%08x.Index%d, data: 0x%08x\n", addr, statusRegister.index, data);
			break;

		case 2:
			//Audio Volume for Left-CD-Out to Right-SPU-Input
			//printf("CDROM - Write Audio Volume Register:\t\t0x%08x.Index%d, data: 0x%08x\n", addr, statusRegister.index, data);
			break;

		case 3:
			//Audio Volume Apply Changes
			//printf("CDROM - Write Audio Volume Register:\t\t0x%08x.Index%d, data: 0x%08x\n", addr, statusRegister.index, data);
			break;
		};
		break;

	default:
		printf("CDROM - Unknown Parameter Set addr:\t\t0x%08x (%d), data: 0x%08x\n", addr, bytes, data);
		return false;
	}

	return true;
}

uint32_t Cdrom::readAddr(uint32_t addr, uint8_t bytes)
{
	auto readDataFifo = [&]()
	{
		uint16_t data;
		uint8_t l, h;

		switch (bytes)
		{
		case 1:
			dataFifo.pop(l);
			data = l;
			break;

		case 2:
			dataFifo.pop(l);
			dataFifo.pop(h);
			data = (h << 8) + l;
			break;
		};

		return (uint32_t)data;
	};

	uint32_t data;
	uint8_t tmp;

	switch (addr)
	{
	case 0x1f801800:
		data = statusRegister.byte;
		//printf("CDROM - Read Status Register:\t\t0x%08x       , data: 0x%08x\n", addr, data);
		break;

	case 0x1f801801:
		responseFifo.pop(tmp);
		data = tmp;
		//printf("CDROM - Read Response Fifo:\t\t0x%08x       , data: 0x%08x\n", addr, data); 
		break;
	
	case 0x1f801802:
		data = readDataFifo();
		//printf("CDROM - Read Data Fifo:\t\t0x%08x           , data: 0x%08x\n", addr, data); 
		break;

	case 0x1f801803:
		switch(statusRegister.index)
		{
		case 0:
			data = interruptEnableRegister;
			//printf("CDROM - Read Int. Enable Register:\t0x%08x.Index%d, data: 0x%08x\n", addr, statusRegister.index, data); 
			break;

		case 1:
			data = 0b11100000 | (interruptFlagRegister & 0x1f);
			//printf("CDROM - Read Int. Flag Register:\t0x%08x.Index%d, data: 0x%08x\n", addr, statusRegister.index, data); 
			break;

		case 2:
			data = interruptEnableRegister;
			//printf("CDROM - Read Int. Enable Register:\t0x%08x.Index%d, data: 0x%08x\n", addr, statusRegister.index, data); 
			break;

		case 3:
			data = 0b11100000 | (interruptFlagRegister & 0x1f);
			//printf("CDROM - Read Int. Flag Register:\t0x%08x.Index%d, data: 0x%08x\n", addr, statusRegister.index, data); 
			break;
		};
		break;

	default:
		printf("CDROM - Unknown Parameter Get addr: 0x%08x (%d)\n", addr, bytes);
		return 0;
	}
	
	return data;
}


//-----------------------------------------------------------------------------------------------
//Commands
//-----------------------------------------------------------------------------------------------
bool Cdrom::cmd_unused() { return false; };

bool Cdrom::cmd_getstat()
{ 
	//Reset OpenShell
	statusCode.shellopen = 0;
	
	//Trigger INT3
	interruptFifo.push(cdrom::INT3);
	responseFifo.push(statusCode.byte);

	return true;
};

bool Cdrom::cmd_setloc() { return false; };
bool Cdrom::cmd_play() { return false; };
bool Cdrom::cmd_forward() { return false; };
bool Cdrom::cmd_backward() { return false; };
bool Cdrom::cmd_readn() { return false; };
bool Cdrom::cmd_motoron() { return false; };
bool Cdrom::cmd_stop() { return false; };
bool Cdrom::cmd_pause() { return false; };
bool Cdrom::cmd_init() { return false; };
bool Cdrom::cmd_mute() { return false; };
bool Cdrom::cmd_demute() { return false; };
bool Cdrom::cmd_setfilter() { return false; };
bool Cdrom::cmd_setmode() { return false; };
bool Cdrom::cmd_getparam() { return false; };
bool Cdrom::cmd_getlocl() { return false; };
bool Cdrom::cmd_getlocp() { return false; };
bool Cdrom::cmd_setsession() { return false; };
bool Cdrom::cmd_gettn() { return false; };
bool Cdrom::cmd_cettd() { return false; };
bool Cdrom::cmd_seekl() { return false; };
bool Cdrom::cmd_seekp() { return false; };

bool Cdrom::cmd_test()
{ 
	//Get All Params
	uint8_t tmp;
	parameterFifo.pop(tmp);

	//Push INT3
	//94h,11h,18h,C0h ;PSX (PU-7); 18 Nov 1994, version vC0 (b)
	interruptFifo.push(cdrom::INT3);
	responseFifo.push( {0x94, 0x11, 0x18, 0xc0} );
	
	return true;	
};

bool Cdrom::cmd_getid()
{ 
	statusCode.spindlemotor = 1;

	//Push INT3(stat)
	interruptFifo.push(cdrom::INT3);
	responseFifo.push(statusCode.byte);

	//Temporary - Empty CD Only
	//Push INT5
	interruptFifo.push(cdrom::INT5);
	responseFifo.push( {0x08, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} );
	//responseFifo.push( {0x08, 0x40, 0x00, 0x00} );
	
	return true;
};

bool Cdrom::cmd_reads() { return false; };
bool Cdrom::cmd_reset() { return false; };
bool Cdrom::cmd_getq() { return false; };
bool Cdrom::cmd_readtoc() { return false; };
bool Cdrom::cmd_videocd() { return false; };
bool Cdrom::cmd_secret() { return false; };
bool Cdrom::cmd_secretlock() { return false; };
bool Cdrom::cmd_crash() { return false; };

//-----------------------------------------------------------------------------------------------
//Sub Functions
//-----------------------------------------------------------------------------------------------



//-----------------------------------------------------------------------------------------------
//Debug Info
//-----------------------------------------------------------------------------------------------

void Cdrom::getDebugInfo(CdromDebugInfo& info)
{
	info.statusRegister = statusRegister.byte;
	info.requestRegister = requestRegister.byte;
	info.interruptEnableRegister = interruptEnableRegister;
	info.interruptFlagRegister = interruptFlagRegister;
}


