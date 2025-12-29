#include <loguru.hpp>
#include "cdrom.h"
#include "psx.h"

Cdrom::Cdrom()
{
	statusCode.byte = 0;
	requestRegister.byte = 0;
	modeRegister.byte = 0;

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

	//Init Internal Counter
	readSectorTimer = 0;

	//Init COMMAND Dictionary
	commandSet = 
	{
		{"Unused", &Cdrom::cmd_unused},
		{"Getstat", &Cdrom::cmd_getstat},
		{"Setloc", &Cdrom::cmd_setloc},
		{"Play", &Cdrom::cmd_play},
		{"Forward", &Cdrom::cmd_forward},
		{"Backward", &Cdrom::cmd_backward},
		{"ReadN", &Cdrom::cmd_readn},
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
	cdImage.closeImage();
}

bool Cdrom::loadImage(const std::string& fileName)
{
	//Load Game Image
	LOG_F(INFO, "PSP Game (%s) Loading...", fileName.c_str());

	if (cdImage.openImage(fileName))
	{
		LOG_F(INFO, "PSP Game Loaded");
		statusCode.shellopen = 0;
		statusCode.spindlemotor = 1;
	}
	else
	{
		LOG_F(ERROR, "PSP Game Not Found");
		statusCode.shellopen = 1;
		statusCode.spindlemotor = 0;
		return false;
	}

	return true;
}

bool Cdrom::reset()
{
	statusCode.byte = 0;
	requestRegister.byte = 0;
	modeRegister.byte = 0;

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

	//Init Internal Counter
	readSectorTimer = 0;
	
	return true;
}

bool Cdrom::execute()
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
		if ((interruptFlagRegister & 0x07) == 0x0)
		{
			cdrom::InterruptEvent* ie;
			ie = interruptFifo.getheadptr();

			//Check if it's time to trigger the Interrupt
			if (ie->delay > 0)
			{
				ie->delay--;
			}
			else
			{
				//Check if Interrupt is enabled
				if ((interruptEnableRegister & ie->interruptNumber) == ie->interruptNumber)
				{
					interruptFlagRegister = (interruptFlagRegister & 0xf8) | ie->interruptNumber;
					//LOG_F(2, "CDROM - Requesting Interrupt [INT%d]", interruptFlagRegister & 0x7);
					LOG_F(INFO, "CDROM - Requesting Interrupt [INT%d]", interruptFlagRegister & 0x7);
					psx->interrupt->set(static_cast<uint32_t>(interruptCause::cdrom));

					//Remove Interrupt from Fifo
					cdrom::InterruptEvent tmp;
					interruptFifo.pop(tmp);
				}
			}
		}
	}

	//Check for available commands
	if (commandAvailable)
	{
		commandAvailable = false;
		statusRegister.busysts = 0;

		//LOG_F(1, "CDROM - Command %s", commandSet[commandRegister].mnemonic.c_str());
		LOG_F(INFO, "CDROM - Command %s", commandSet[commandRegister].mnemonic.c_str());
		bResult = (this->*commandSet[commandRegister].operate)();
			if (!bResult)
				LOG_F(ERROR, "CDROM - Unimplemented Command %s!", commandSet[commandRegister].mnemonic.c_str());
	}

	//Read Sector in Read or Play mode
	if (statusCode.play == 1 || statusCode.read == 1)
	{
		//Read Speed is 75 Sector per Second at x1 speed and 150 Sector per Second at x2 speed
		//CDROM Clock speed is 16.9344 MHz, a Sector is read every:
		//   - 16934400/75 = 225792 clock tick at x1 speed
		//   - 16934400/150 = 112896 clock tick at x2 speed
		//TODO - Only works for READN Command
		if (readSectorTimer)
		{
			readSectorTimer--;
		}
		else
		{
			char sectorData[sector_payload_mode1_size];

			//Push INT1(stat)
			cdrom::InterruptEvent ie;
			ie.delay = 0;
			ie.interruptNumber = cdrom::INT1;
			interruptFifo.push(ie);
			responseFifo.push(statusCode.byte);

			cdImage.readSector(sectorData);

			for(int i = 0; i < sector_payload_mode1_size;  i++)
				dataFifo.push((uint8_t)sectorData[i]);
			
			//Set Reading Speed according to actual mode
			readSectorTimer = 16934400/(75*(modeRegister.speed + 1));
		}
	}

	return true;
}

bool Cdrom::writeAddr(uint32_t addr, uint32_t& data, uint8_t bytes)
{
	switch (addr)
	{
	case 0x1f801800:
		LOG_F(INFO, "CDROM - Write Status Register:\t0x%08x       , data: 0x%08x", addr, data);
		//LOG_F(3, "CDROM - Write Status Register:\t\t0x%08x       , data: 0x%08x", addr, data);
		statusRegister.index = data & 0x3;
		break;
	
	case 0x1f801801:
		switch(statusRegister.index)
		{
		case 0:
			commandRegister = data;
			commandAvailable = true;
			statusRegister.busysts = 1;
			LOG_F(INFO, "CDROM - Write Command Register:\t0x%08x.Index%d, data: 0x%08x", addr, statusRegister.index, data);
			//LOG_F(3, "CDROM - Write Command Register:\t\t0x%08x.Index%d, data: 0x%08x", addr, statusRegister.index, data);
			break;

		case 1:
			//Sound Map Data Out
			LOG_F(INFO, "CDROM - Write Sound Map Data Register:\t\t0x%08x.Index%d, data: 0x%08x", addr, statusRegister.index, data);
			//LOG_F(3, "CDROM - Write Sound Map Data Register:\t\t0x%08x.Index%d, data: 0x%08x", addr, statusRegister.index, data);
			break;

		case 2:
			//Sound Map Coding Info
			LOG_F(INFO, "CDROM - Write Sound Map Coding Register:\t\t0x%08x.Index%d, data: 0x%08x", addr, statusRegister.index, data);
			//LOG_F(3, "CDROM - Write Sound Map Coding Register:\t\t0x%08x.Index%d, data: 0x%08x", addr, statusRegister.index, data);
			break;

		case 3:
			//Audio Volume for Right-CD-Out to Right-SPU-Input
			LOG_F(INFO, "CDROM - Write Audio Volume Register:\t\t0x%08x.Index%d, data: 0x%08x", addr, statusRegister.index, data);
			//LOG_F(3, "CDROM - Write Audio Volume Register:\t\t0x%08x.Index%d, data: 0x%08x", addr, statusRegister.index, data);
			break;
		};
		break;

	case 0x1f801802:
		switch(statusRegister.index)
		{
		case 0:
			parameterFifo.push(data);
			LOG_F(INFO, "CDROM - Write Parameter Fifo:\t\t0x%08x.Index%d, data: 0x%08x", addr, statusRegister.index, data);
			//LOG_F(3, "CDROM - Write Parameter Fifo:\t\t0x%08x.Index%d, data: 0x%08x", addr, statusRegister.index, data);
			break;

		case 1:
			interruptEnableRegister = data & 0x1f;
			LOG_F(INFO, "CDROM - Write Int. Enable Register:\t0x%08x.Index%d, data: 0x%08x", addr, statusRegister.index, data);
			//LOG_F(3, "CDROM - Write Int. Enable Register:\t0x%08x.Index%d, data: 0x%08x", addr, statusRegister.index, data);
			break;

		case 2:
			//Audio Volume for Left-CD-Out to Left-SPU-Input
			LOG_F(INFO, "CDROM - Write Audio Volume Register:\t\t0x%08x.Index%d, data: 0x%08x", addr, statusRegister.index, data); 
			//LOG_F(3, "CDROM - Write Audio Volume Register:\t\t0x%08x.Index%d, data: 0x%08x", addr, statusRegister.index, data); 
			break;

		case 3:
			//Audio Volume for Right-CD-Out to Left-SPU-Input
			LOG_F(INFO, "CDROM - Write Audio Volume Register:\t\t0x%08x.Index%d, data: 0x%08x", addr, statusRegister.index, data);
			//LOG_F(3, "CDROM - Write Audio Volume Register:\t\t0x%08x.Index%d, data: 0x%08x", addr, statusRegister.index, data);
			break;
		};
		break;

		case 0x1f801803:
		switch(statusRegister.index)
		{
		case 0:
			requestRegister.byte = data & 0xe0;
			//If RequestRegister.bit7 = 0, Reset DataFifo
			//if (requestRegister.bfrd == 0) dataFifo.flush();
			LOG_F(INFO, "CDROM - Write Request Register:\t0x%08x.Index%d, data: 0x%08x", addr, statusRegister.index, data);
			//LOG_F(3, "CDROM - Write Request Register:\t\t0x%08x.Index%d, data: 0x%08x", addr, statusRegister.index, data);
			break;

		case 1:
			interruptFlagRegister = (data & 0xe0) | ((interruptFlagRegister & 0x1f) & ~(data & 0x1f));
			if (data & 0x40)
				parameterFifo.flush();

			LOG_F(INFO, "CDROM - Write Int. Flag Register:\t0x%08x.Index%d, data: 0x%08x", addr, statusRegister.index, data);	
			//LOG_F(3, "CDROM - Write Int. Flag Register:\t0x%08x.Index%d, data: 0x%08x", addr, statusRegister.index, data);
			break;

		case 2:
			//Audio Volume for Left-CD-Out to Right-SPU-Input
			LOG_F(INFO, "CDROM - Write Audio Volume Register:\t0x%08x.Index%d, data: 0x%08x", addr, statusRegister.index, data);
			//LOG_F(3, "CDROM - Write Audio Volume Register:\t\t0x%08x.Index%d, data: 0x%08x", addr, statusRegister.index, data);
			break;

		case 3:
			//Audio Volume Apply Changes
			LOG_F(INFO, "CDROM - Write Audio Volume Register:\t0x%08x.Index%d, data: 0x%08x", addr, statusRegister.index, data);
			//LOG_F(3, "CDROM - Write Audio Volume Register:\t\t0x%08x.Index%d, data: 0x%08x", addr, statusRegister.index, data);
			break;
		};
		break;

	default:
		LOG_F(ERROR, "CDROM - Unknown Parameter Set addr:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
		return false;
	}

	return true;
}

uint32_t Cdrom::readAddr(uint32_t addr, uint8_t bytes)
{
	uint32_t data = 0;
	uint8_t tmp;

	switch (addr)
	{
	case 0x1f801800:
		data = statusRegister.byte;
		LOG_F(INFO, "CDROM - Read Status Register:\t\t0x%08x       , data: 0x%08x", addr, data);
		//LOG_F(3, "CDROM - Read Status Register:\t\t0x%08x       , data: 0x%08x", addr, data);
		break;

	case 0x1f801801:
		if (responseFifo.pop(tmp))
			data = tmp;
		LOG_F(INFO, "CDROM - Read Response Fifo:\t\t0x%08x       , data: 0x%02x", addr, data); 	
		//LOG_F(3, "CDROM - Read Response Fifo:\t\t0x%08x       , data: 0x%02x", addr, data); 
		break;
	
	case 0x1f801802:
		if (dataFifo.pop(tmp))
			data = tmp;
		LOG_F(3, "CDROM - Read Data Fifo:\t\t0x%08x       , data: 0x%08x", addr, data); 	
		break;

	case 0x1f801803:
		switch(statusRegister.index)
		{
		case 0:
			data = interruptEnableRegister;
			LOG_F(INFO, "CDROM - Read Int. Enable Register:\t0x%08x.Index%d, data: 0x%08x", addr, statusRegister.index, data); 
			//LOG_F(3, "CDROM - Read Int. Enable Register:\t0x%08x.Index%d, data: 0x%08x", addr, statusRegister.index, data); 
			break;

		case 1:
			data = 0b11100000 | (interruptFlagRegister & 0x1f);
			LOG_F(INFO, "CDROM - Read Int. Flag Register:\t0x%08x.Index%d, data: 0x%08x", addr, statusRegister.index, data); 
			//LOG_F(3, "CDROM - Read Int. Flag Register:\t0x%08x.Index%d, data: 0x%08x", addr, statusRegister.index, data); 
			break;

		case 2:
			data = interruptEnableRegister;
			LOG_F(INFO, "CDROM - Read Int. Enable Register:\t0x%08x.Index%d, data: 0x%08x", addr, statusRegister.index, data); 
			//LOG_F(3, "CDROM - Read Int. Enable Register:\t0x%08x.Index%d, data: 0x%08x", addr, statusRegister.index, data); 
			break;

		case 3:
			data = 0b11100000 | (interruptFlagRegister & 0x1f);
			LOG_F(INFO, "CDROM - Read Int. Flag Register:\t0x%08x.Index%d, data: 0x%08x", addr, statusRegister.index, data);
			//LOG_F(3, "CDROM - Read Int. Flag Register:\t0x%08x.Index%d, data: 0x%08x", addr, statusRegister.index, data); 
			break;
		};
		break;

	default:
		LOG_F(ERROR, "CDROM - Unknown Parameter Get addr: 0x%08x (%d)", addr, bytes);
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
	
	//Acknowledge with INT3(stat)
	cdrom::InterruptEvent ie;
	ie.delay = 0; //Temporary Value;
	ie.interruptNumber = cdrom::INT3;
	interruptFifo.push(ie);	
	responseFifo.push(statusCode.byte);

	return true;
};

bool Cdrom::cmd_setloc()
{ 
	//Get All Parameters
	uint8_t amm, ass, asect;
	parameterFifo.pop(amm);
	parameterFifo.pop(ass);
	parameterFifo.pop(asect);
	LOG_F(INFO, "CDROM - Command Parameters [amm: %d, ass: %d, asect: %d]", amm, ass, asect);
	//LOG_F(1, "CDROM - Command Parameters [amm %d, ass %d, asect %d]", amm, ass, asect);

	//Search Sector on CdRom Image
	cdImage.setLocation(amm, ass, asect); 

	//Acknowledge with INT3(stat)
	cdrom::InterruptEvent ie;
	ie.delay = 0; //Temporary Value;
	ie.interruptNumber = cdrom::INT3;
	interruptFifo.push(ie);	
	responseFifo.push(statusCode.byte);

	return true;
};

bool Cdrom::cmd_play() { return false; };
bool Cdrom::cmd_forward() { return false; };
bool Cdrom::cmd_backward() { return false; };

bool Cdrom::cmd_readn()
{ 
	//Acknowledge with INT3(stat)
	cdrom::InterruptEvent ie;
	ie.delay = 0; //Temporary Value;
	ie.interruptNumber = cdrom::INT3;
	interruptFifo.push(ie);
	
	//Reset Seek and Set Read
	statusCode.play = 0;
	statusCode.seek = 0;
	statusCode.read = 1;
	responseFifo.push(statusCode.byte);
	
	//Set Reading Speed according to actual mode
	readSectorTimer = 16934400/(75*(modeRegister.speed + 1));
	
	return true;
};

bool Cdrom::cmd_motoron() { return false; };
bool Cdrom::cmd_stop() { return false; };

bool Cdrom::cmd_pause()
{	
	//Acknowledge with INT3(stat)
	cdrom::InterruptEvent ie;
	ie.delay = 0; //Temporary Value;
	ie.interruptNumber = cdrom::INT3;
	interruptFifo.push(ie);
	responseFifo.push(statusCode.byte);

	//Command COmplete with INT2(stat)
	ie.delay = 0; //Temporary Value;
	ie.interruptNumber = cdrom::INT2;
	interruptFifo.push(ie);

	//Reset Read
	statusCode.play = 0;
	statusCode.seek = 0;
	statusCode.read = 0;
	responseFifo.push(statusCode.byte);

	return true;
};
bool Cdrom::cmd_init() { return false; };
bool Cdrom::cmd_mute() { return false; };
bool Cdrom::cmd_demute() { return false; };
bool Cdrom::cmd_setfilter() { return false; };

bool Cdrom::cmd_setmode()
{ 
	//Get All Parameters
	uint8_t mode;
	parameterFifo.pop(mode);

	LOG_F(INFO, "CDROM - Command Parameters [mode: 0x%02x]", mode);
	//LOG_F(1, "CDROM - Command Parameters [mode %d]", mode);
	
	//Update Mode Register
	modeRegister.byte = mode;

	//Acnowkledge with INT3(stat)
	cdrom::InterruptEvent ie;
	ie.delay = 0; //Temporary Value;
	ie.interruptNumber = cdrom::INT3;
	interruptFifo.push(ie);
	responseFifo.push(statusCode.byte);

	return true;
};

bool Cdrom::cmd_getparam() { return false; };
bool Cdrom::cmd_getlocl() { return false; };
bool Cdrom::cmd_getlocp() { return false; };
bool Cdrom::cmd_setsession() { return false; };
bool Cdrom::cmd_gettn() { return false; };
bool Cdrom::cmd_cettd() { return false; };

bool Cdrom::cmd_seekl()
{
	//Acknowledge with INT3(stat)
	cdrom::InterruptEvent ie;
	ie.delay = 0; //Temporary Value;
	ie.interruptNumber = cdrom::INT3;
	interruptFifo.push(ie);
	responseFifo.push(statusCode.byte);

	//Search Sector on CdRom Image
	cdImage.seekSector();

	//Complete Command with INT2(stat)
	ie.delay = 0; //Temporary Value;
	ie.interruptNumber = cdrom::INT2;
	interruptFifo.push(ie);

	//Status Seek
	statusCode.play = 0;
	statusCode.seek = 1;
	statusCode.read = 0;
	responseFifo.push(statusCode.byte);

	return true;

};

bool Cdrom::cmd_seekp() { return false; };

bool Cdrom::cmd_test()
{ 
	//Get All Params
	uint8_t sub;
	parameterFifo.pop(sub);

	LOG_F(INFO, "CDROM - Command Parameters [sub: 0x%02x]", sub);
	//LOG_F(1, "CDROM - Command Parameters [param1 %02x]", tmp);

	cdrom::InterruptEvent ie;
	switch(sub)
	{
		case 0x20:  //CDROM Controller Version
			//Acknowledge with INT3(stat)
			ie.delay = 0; //Temporary Value;
			ie.interruptNumber = cdrom::INT3;
			interruptFifo.push(ie);
			responseFifo.push( {0x94, 0x09, 0x19, 0xc0} );
			break;
		default:
			//Unknown Sub Command
			LOG_F(ERROR, "CDROM - Unknown TEST Sub Command 0x%02x", sub);
	}
	
	return true;	
};

bool Cdrom::cmd_getid()
{ 
	/*
	static int count = 0;
	if (count < 2)
	{
		//Not Ready
		interruptFifo.push(cdrom::INT5);
		switch (count)
		{
			case 0:
				responseFifo.push( {0x11, 0x80} );
				break;
			case 1:
				responseFifo.push({ 0x03, 0x80 });
				break;
		}
		count++;

		return true;
	}
	*/
	
	//Acknowledge with INT3(stat)
	cdrom::InterruptEvent ie;
	ie.delay = 0; //Temporary Value;
	ie.interruptNumber = cdrom::INT3;
	interruptFifo.push(ie);
	responseFifo.push(statusCode.byte);

	//Complete Command with INT2(stat)
	ie.delay = 0; //Temporary Value;
	ie.interruptNumber = cdrom::INT2;
	interruptFifo.push(ie);
	responseFifo.push( {0x02, 0x00, 0x20, 0x00, 0x53, 0x43, 0x45, 0x41} ); //SCEA 0x41, SCEE 0x45
	
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

