#include <loguru.hpp>
#include "cdrom.h"
#include "psx.h"

Cdrom::Cdrom()
{
	//Initialize Internal Registers
	requestRegister.byte = 0;
	modeRegister.byte = 0;
	interruptMaskRegister.byte = 0;
	interruptStatusRegister.byte = 0;
	
	commandFifo.flush();
	adpcmFifo.flush();
	parameterFifo.flush();
	dataFifo.flush();
	responseFifo.flush();
	interruptFifo.flush();

	//Init Internal Status
	cdImageLoaded = false;	
	cdShellOpen = false;
	cdMotorOn = false;
	isStreamingData = false;
	streamingINT1Delay = 0;

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
		cdImageLoaded = true;
		cdShellOpen = false;
		cdMotorOn = true;
	}
	else
	{
		LOG_F(ERROR, "PSP Game Not Found");
		cdImageLoaded = false;
		cdShellOpen = false;
		cdMotorOn = false;
		return false;
	}

	LOG_F(2, "CDR - Offset: %u, Mode: %u, Form: %u", 
          cdImage.getSectorOffset(), cdImage.getSectorInfo().mode, cdImage.getSectorInfo().form);

	return true;
}

bool Cdrom::reset()
{
	//Initialize Internal Registers
	requestRegister.byte = 0;
	modeRegister.byte = 0;
	interruptMaskRegister.byte = 0;
	interruptStatusRegister.byte = 0;
	
	commandFifo.flush();
	adpcmFifo.flush();
	parameterFifo.flush();
	dataFifo.flush();
	responseFifo.flush();
	interruptFifo.flush();

	//Init Internal Status
	cdShellOpen = false;
	cdMotorOn = false;
	isStreamingData = false;
	streamingINT1Delay = 0;
	
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
		if ((interruptStatusRegister.byte & 0x1f) == 0x00)
		{
			//Get Interrupt Event at the head of the Fifo
			cdrom::InterruptEvent* ie;
			ie = interruptFifo.getheadptr();

			//Check if it's time to trigger the Interrupt
			if (ie->delay > 0)
			{
				ie->delay--;
			}
			else
			{
				//Check if the current Interrupt is enabled
				if ((interruptMaskRegister.enint & ie->interruptNumber) == ie->interruptNumber)
				{
					//Reset busysts in case of INT3 (Command Complete)
					if (ie->interruptNumber == cdrom::INT3)
						statusRegister.busysts = 0;
					
					//Set Interrupt Status
					interruptStatusRegister.intsts = ie->interruptNumber;
					LOG_F(2, "CDR - Requesting Interrupt [INT%d]", interruptStatusRegister.intsts & 0x07);
					psx->interrupt->request(static_cast<uint32_t>(interrupt::Cause::cdrom));

					//Remove Interrupt from Fifo
					cdrom::InterruptEvent tmp;
					interruptFifo.pop(tmp);
				}

				
			}
		}
	}

	//Check for available commands
	if (!commandFifo.isempty())
	{
		uint8_t command;

		//Extract Commanf from FIFO
		commandFifo.pop(command);

		LOG_F(1, "CDR - Command %s", commandSet[command].mnemonic.c_str());
		bResult = (this->*commandSet[command].operate)();
			if (!bResult)
				LOG_F(ERROR, "CDR - Unimplemented Command %s!", commandSet[command].mnemonic.c_str());
	}

	//If we are in Streaming mode, continue reading sectors and respond with INT1
	if (isStreamingData)
	{
		streamingINT1Delay--;
		if (streamingINT1Delay == 0)
		{
			int sectorSize = (modeRegister.sector_size) ? payload_size_raw : payload_size_mode2;
			char sectorData[total_sector_size];

			//Read Sector
			cdImage.readSector(sectorData);  //TODO read always 2048 bytes, add supporto for raw size.

			//Push Sector Data to Data Fifo
			for(int i = 0; i < sectorSize;  i++)
				dataFifo.push((uint8_t)sectorData[i]);

			//Push INT1(stat)
			cdrom::InterruptEvent ie;
			ie.delay = 0; //Fire immediately, delay is already handled by streamingINT1Delay
			ie.interruptNumber = cdrom::INT1;
			interruptFifo.push(ie);

			//Prepare Response and push to Response Fifo
			cdrom::StatusCode sc;
			sc.byte = 0;
			sc.spindlemotor = cdMotorOn;
			sc.shellopen = cdShellOpen;
			sc.read = 1;
			responseFifo.push(sc.byte);
			
			//Reset Streaming INT1 Delay for next sector
			streamingINT1Delay = modeRegister.speed ? 112900 : 225800; //Set delay according to speed mode
		}
	}

	return true;
}

bool Cdrom::writeAddr(uint32_t addr, uint32_t& data, uint8_t bytes)
{
	switch (addr)
	{
	case 0x1f801800:
		LOG_F(3, "CDR - Write HSTS Register:\t\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
		statusRegister.index = data & 0x3;
		break;
	
	case 0x1f801801:
		switch(statusRegister.index)
		{
		case 0:
			commandFifo.push(data);
			statusRegister.busysts = 1;
			LOG_F(3, "CDR - Write COMMAND Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
			break;

		case 1:
			//Sound Map Data Out
			LOG_F(3, "CDR - Write WRDATA Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
			break;

		case 2:
			//Sound Map Coding Info
			LOG_F(3, "CDR - Write CI Register:\t\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
			break;

		case 3:
			//Audio Volume for Right-CD-Out to Right-SPU-Input
			LOG_F(3, "CDR - Write ATV2 Register:\t\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
			break;
		};
		break;

	case 0x1f801802:
		switch(statusRegister.index)
		{
		case 0:
			parameterFifo.push(data);
			LOG_F(3, "CDR - Write PARAMETER Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
			break;

		case 1:
			//Write Interrupt Mask Register
			interruptMaskRegister.byte = data & 0x1f;
			LOG_F(3, "CDR - Write HINTMSK Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
			break;

		case 2:
			//Audio Volume for Left-CD-Out to Left-SPU-Input
			LOG_F(3, "CDR - Write ATV0 Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data); 
			break;

		case 3:
			//Audio Volume for Right-CD-Out to Left-SPU-Input
			LOG_F(3, "CDR - Write ATV3 Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
			break;
		};
		break;

		case 0x1f801803:
		switch(statusRegister.index)
		{
		case 0:
			//Write Request Register, only top 3 bits are used the other are always 0
			requestRegister.byte = data & 0xe0;
			LOG_F(3, "CDR - Write HCHPCTL Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
			break;

		case 1:
			//Update Interrupt Status Register
			interruptStatusRegister.byte = (interruptStatusRegister.byte & 0xe0) | 
			                               ((interruptStatusRegister.byte & 0x1f) & ~(data & 0x1f));

			//If bit 5 is set, clear sound map XA-ADPCM buffer
			if (data & 0x20)
				adpcmFifo.flush();

			//If bit 6 is set, clear parameter fifo
			if (data & 0x40)
				parameterFifo.flush();
			
			//if bit 7 is set, reset decoder chip
				//TODO - implement decoder reset

			LOG_F(3, "CDR - Write HCLRCTL Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);	
			break;

		case 2:
			//Audio Volume for Left-CD-Out to Right-SPU-Input
			LOG_F(3, "CDR - Write ATV1 Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
			break;

		case 3:
			//Audio Volume Apply Changes
			LOG_F(3, "CDR - Write ADPCTL Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
			break;
		};
		break;

	default:
		LOG_F(ERROR, "CDR - Write Unknown Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
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
		LOG_F(3, "CDR - Read HSTS Register:\t\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
		break;

	case 0x1f801801:
		if (responseFifo.pop(tmp))
			data = tmp;
		LOG_F(3, "CDR - Read RESULT Fifo:\t\t0x%08x (%d), data: 0x%02x", addr, bytes, data); 	 
		break;
	
	case 0x1f801802:
		if (dataFifo.pop(tmp))
			data = tmp;
		LOG_F(3, "CDR - Read RDDATA Register:\t\t0x%08x (%d), data: 0x%08x", addr, bytes, data); 	
		break;

	case 0x1f801803:
		switch(statusRegister.index)
		{
		case 0:
		case 2:
			data = interruptMaskRegister.byte | 0xe0; //top 3 bits are unused, read return always 1
			LOG_F(3, "CDR - Read HINTMSK Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);  
			break;

		case 1:
		case 3:
			data = interruptStatusRegister.byte | 0xe0; //top 3 bits are unused, read return always 1
			LOG_F(3, "CDR - Read HINTSTS Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);  
			break;
		};
		break;

	default:
		LOG_F(ERROR, "CDR - Read Unknown Register:\t 0x%08x (%d), data: 0x%08x", addr, bytes, data);
		return 0x0;
	}
	
	return data;
}


//-----------------------------------------------------------------------------------------------
//Commands
//-----------------------------------------------------------------------------------------------
bool Cdrom::cmd_unused() { return false; };

bool Cdrom::cmd_getstat()
{ 
	cdrom::StatusCode sc;
	
	//Acknowledge with INT3(stat)
	cdrom::InterruptEvent ie;
	ie.delay = 102; //about 6us
	ie.interruptNumber = cdrom::INT3;
	interruptFifo.push(ie);	

	//Prepare Status Code and push to Response Fifo
	sc.byte = 0;
	sc.shellopen = cdShellOpen; 		
	sc.spindlemotor = cdMotorOn;
	responseFifo.push(sc.byte);

	return true;
};

bool Cdrom::cmd_setloc()
{ 
	cdrom::StatusCode sc;

	//Get All Parameters
	uint8_t amm, ass, asect;
	parameterFifo.pop(amm);
	parameterFifo.pop(ass);
	parameterFifo.pop(asect);
	LOG_F(1, "CDR - Command Parameters [mm: 0x%02x, ss: 0x%02x, ff: 0x%02x]", amm, ass, asect);

	//Search Sector on CdRom Image
	cdImage.setLocation(amm, ass, asect);
	LOG_F(1, "CDR - Sector Target: %d", cdImage.getSectorTarget());

	//Acknowledge with INT3(stat)
	cdrom::InterruptEvent ie;
	ie.delay = 102; //about 6us
	ie.interruptNumber = cdrom::INT3;
	interruptFifo.push(ie);

	//Prepare Status Code and push to Response Fifo
	sc.byte = 0;
	sc.shellopen = cdShellOpen;
	sc.spindlemotor = cdMotorOn; 
	responseFifo.push(sc.byte);

	return true;
};

bool Cdrom::cmd_play() { return false; };
bool Cdrom::cmd_forward() { return false; };
bool Cdrom::cmd_backward() { return false; };

bool Cdrom::cmd_readn()
{ 
	cdrom::StatusCode sc;

	//Acknowledge with INT3(stat)
	cdrom::InterruptEvent ie;
	ie.delay = 102; //about 6us
	ie.interruptNumber = cdrom::INT3;
	interruptFifo.push(ie);

	//Prepare Status Code and push to Response Fifo
	sc.byte = 0;
	sc.shellopen = cdShellOpen;
	sc.spindlemotor = cdMotorOn;
	sc.read = 1;
	responseFifo.push(sc.byte);

	//Set the CDROM in Streaming mode, receives sectors continuously until Pause/Stop command
	isStreamingData = true;
	streamingINT1Delay = modeRegister.speed ? 112900 : 225800; //Set delay according to speed mode

	return true;
};

bool Cdrom::cmd_motoron() { return false; };
bool Cdrom::cmd_stop() { return false; };

bool Cdrom::cmd_pause()
{	
	cdrom::StatusCode sc;

	//Acknowledge with INT3(stat)
	cdrom::InterruptEvent ie;
	ie.delay = 102; //about 6us
	ie.interruptNumber = cdrom::INT3;
	interruptFifo.push(ie);

	//Prepare Status Code and push to Response Fifo
	sc.byte = 0;
	sc.spindlemotor = cdMotorOn;
	sc.shellopen = cdShellOpen;
	sc.read = 1;  //TODO, support forn play/pause status
	responseFifo.push(sc.byte);

	//Command COmplete with INT2(stat)
	ie.delay = modeRegister.speed ? (112900/2) : (225800/2); //Set delay according to speed mode, assuming it occurs after half sector read
	ie.interruptNumber = cdrom::INT2;
	interruptFifo.push(ie);

	//Prepare Status Code and push to Response Fifo
	sc.byte = 0;
	sc.spindlemotor = cdMotorOn;
	sc.shellopen = cdShellOpen;
	sc.read  = 0;
	responseFifo.push(sc.byte);

	//Stop Streaming Data
	isStreamingData = false;

	return true;
};
bool Cdrom::cmd_init() { return false; };
bool Cdrom::cmd_mute() { return false; };
bool Cdrom::cmd_demute() { return false; };
bool Cdrom::cmd_setfilter() { return false; };

bool Cdrom::cmd_setmode()
{ 
	cdrom::StatusCode sc;

	//Get All Parameters
	uint8_t mode;
	parameterFifo.pop(mode);

	LOG_F(1, "CDR - Command Parameters [mode: 0x%02x]", mode);
	
	//Update Mode Register
	modeRegister.byte = mode;

	//Acnowkledge with INT3(stat)
	cdrom::InterruptEvent ie;
	ie.delay = 102; //about 6us
	ie.interruptNumber = cdrom::INT3;
	interruptFifo.push(ie);

	//Prepare Status Code and push to Response Fifo
	sc.byte = 0;
	sc.shellopen = cdShellOpen;
	sc.spindlemotor = cdMotorOn;
	responseFifo.push(sc.byte);

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
	cdrom::StatusCode sc;

	//Acknowledge with INT3(stat)
	cdrom::InterruptEvent ie;
	ie.delay = 102; //about 6us
	ie.interruptNumber = cdrom::INT3;
	interruptFifo.push(ie);

	//Prepare Status Code and push to Response Fifo
	sc.byte = 0;
	sc.shellopen = cdShellOpen;
	sc.spindlemotor = cdMotorOn;
	sc.seek = 1;
	responseFifo.push(sc.byte);

	//Search Sector on CdRom Image according to sector target set by setloc
	cdImage.seekSector();

	//Complete Command with INT2(stat)
	ie.delay = 677500; //about 40ms
	ie.interruptNumber = cdrom::INT2;
	interruptFifo.push(ie);

	//Prepare Status Code and push to Response Fifo
	sc.byte = 0;
	sc.shellopen = cdShellOpen;
	sc.spindlemotor = cdMotorOn;
	responseFifo.push(sc.byte);

	return true;

};

bool Cdrom::cmd_seekp() { return false; };

bool Cdrom::cmd_test()
{ 
	//Get All Params
	uint8_t sub;
	parameterFifo.pop(sub);

	LOG_F(1, "CDR - Command Parameters [sub: 0x%02x]", sub);
	
	cdrom::InterruptEvent ie;
	switch(sub)
	{
		case 0x20:  //CDROM Controller Version
			//Acknowledge with INT3(stat)
			ie.delay = 68000; //about 4ms
			ie.interruptNumber = cdrom::INT3;
			interruptFifo.push(ie);
			responseFifo.push( {0x94, 0x09, 0x19, 0xc0} );
			break;
		default:
			//Unknown Sub Command
			LOG_F(ERROR, "CDR - Unknown TEST Sub Command 0x%02x", sub);
	}
	
	return true;	
};

bool Cdrom::cmd_getid()
{ 
	cdrom::StatusCode sc;

	//Acknowledge with INT3(stat)
	cdrom::InterruptEvent ie;
	ie.delay = 102; //about 6 us
	ie.interruptNumber = cdrom::INT3;
	interruptFifo.push(ie);

	//Prepare Status Code and push to Response Fifo
	sc.byte = 0;
	sc.shellopen = cdShellOpen;
	sc.spindlemotor = cdMotorOn;
	responseFifo.push(sc.byte);

	//Complete Command with INT2(stat)
	ie.delay = 1500; //about 80 us
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

