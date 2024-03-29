#pragma once

#include <cstdint>
#include <cstdio>
#include <memory>
#include <vector>
#include <string>

#include "fifo.h"
#include "libcdimage.h"

namespace cdrom
{
	union StatusCode
	{
		uint8_t byte;

		struct
		{
			uint8_t error : 1;
			uint8_t spindlemotor : 1;
			uint8_t seekerror : 1;
			uint8_t iderror : 1;
			uint8_t shellopen : 1;
			uint8_t read : 1;
			uint8_t seek : 1;
			uint8_t play : 1;
		};
		
	};

	union StatusRegister
	{
		uint8_t		byte;

		struct
		{
			uint8_t index : 2;		//Port 1F801801h-1F801803h index (0..3 = Index0..Index3)   (R/W)
			uint8_t adpbusy : 1;	//XA-ADPCM fifo empty  (0=Empty) ;set when playing XA-ADPCM sound
			uint8_t prmempt : 1;	//Parameter fifo empty (1=Empty) ;triggered before writing 1st byte
			uint8_t prmwrdy : 1;	//Parameter fifo full  (0=Full)  ;triggered after writing 16 bytes
			uint8_t rslrrdy : 1;	//Response fifo empty  (0=Empty) ;triggered after reading LAST byte
			uint8_t drqsts : 1;		//Data fifo empty      (0=Empty) ;triggered after reading LAST byte
			uint8_t busysts : 1;	//Command/parameter transmission busy  (1=Busy)
		};
	};

	union RequestRegister
	{
		uint8_t byte;

		struct
		{
			uint8_t unused : 5;
			uint8_t smen : 1;
			uint8_t bfwr : 1;
			uint8_t bfrd : 1;
		};
	};

	union ModeRegister
	{
		uint8_t		byte;

		struct
		{
			uint8_t	cdda : 1;
			uint8_t autopause : 1;
			uint8_t	report : 1;
			uint8_t xa_filter : 1;
			uint8_t ignore_bit : 1;
			uint8_t sector_size : 1;
			uint8_t xa_adpcm : 1;
			uint8_t speed : 1;
		};
	};

	enum {INT1 = 1, INT2 = 2, INT3 = 3, INT5 = 5};
};


struct CdromDebugInfo
{
	uint8_t		statusRegister;
	uint8_t		requestRegister;
	uint8_t		interruptEnableRegister;
	uint8_t		interruptFlagRegister;
};

class Psx;

class Cdrom
{
public:
	Cdrom();
	~Cdrom();

	bool reset();
	bool execute();
	bool loadImage(const std::string& fileName);

	bool writeAddr(uint32_t addr, uint32_t& data, uint8_t bytes);
	uint32_t readAddr(uint32_t addr, uint8_t bytes);

	//Debug Info
	void getDebugInfo(CdromDebugInfo& info);

	//Connect to PSX Instance
	void link(Psx* instance) { psx = instance; }

private:
	//Link to Bus Object
	Psx* psx;
	//CD Image Object
	CdImage	cdImage;

	//Internal Status
	bool commandAvailable;

	//Internal Registers
	cdrom::StatusCode		statusCode;
	cdrom::StatusRegister	statusRegister;
	cdrom::RequestRegister	requestRegister;
	cdrom::ModeRegister		modeRegister;
	uint8_t					interruptEnableRegister;
	uint8_t					interruptFlagRegister;
	uint8_t					commandRegister;

	//Internat Counter
	uint32_t				readSectorTimer;

	//Internal Fifo
	lite::fifo<uint8_t, 16> 	parameterFifo;
	lite::fifo<uint8_t, 2048 * 2> 	dataFifo;
	lite::fifo<uint8_t, 16> 	responseFifo;
	lite::fifo<uint8_t, 16> 	interruptFifo;
	lite::fifo<uint8_t, 2048 * 2> 	adpcmFifo;

	//Command Functions
	bool cmd_unused();
	bool cmd_getstat();
	bool cmd_setloc();
	bool cmd_play();
	bool cmd_forward();
	bool cmd_backward();
	bool cmd_readn();
	bool cmd_motoron();
	bool cmd_stop();
	bool cmd_pause();
	bool cmd_init();
	bool cmd_mute();
	bool cmd_demute();
	bool cmd_setfilter();
	bool cmd_setmode();
	bool cmd_getparam();
	bool cmd_getlocl();
	bool cmd_getlocp();
	bool cmd_setsession();
	bool cmd_gettn();
	bool cmd_cettd();
	bool cmd_seekl();
	bool cmd_seekp();
	bool cmd_test();
	bool cmd_getid();
	bool cmd_reads();
	bool cmd_reset();
	bool cmd_getq();
	bool cmd_readtoc();
	bool cmd_videocd();
	bool cmd_secret();
	bool cmd_secretlock();
	bool cmd_crash();


	//Full set of CPU Instruction Dictionaries
	struct COMMAND
	{
		std::string mnemonic;
		bool(Cdrom::* operate)() = nullptr;
	};

	std::vector<COMMAND> commandSet;	//Full Command Set
	std::vector<COMMAND> functSet;		//Full Sub Functions Set
};

