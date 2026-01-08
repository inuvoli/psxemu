#pragma once

#include <cstdint>
#include <cstdio>
#include <memory>
#include <vector>
#include <string>

#include "fifo.h"
#include "libcdimage.h"

constexpr auto total_sector_size = 2352;
constexpr auto payload_size_mode2 = 2048;
constexpr auto payload_size_raw = 2340;

namespace cdrom
{
	struct InterruptEvent
	{
		uint32_t delay; 				//Delay in number of CDROM Clock Cycles
		uint8_t	interruptNumber; 		//Interrupt Number to trigger
	};

	union StatusCode
	{
		uint8_t byte;

		struct
		{
			uint8_t error : 1;		 	//Invalid Command/Parameter (followed by INT5 Error Byte)
			uint8_t spindlemotor : 1;	//Spindle Motor On/Off (0=Off or in spin-up, 1=On)
			uint8_t seekerror : 1;		//Seek Error Occurred (0=OK, 1=Error - followed by INT5 Error Byte)
			uint8_t iderror : 1;		//GetID Error Occurred (0=OK, 1=Error - also set when Setmode.bit4 is 1)
			uint8_t shellopen : 1;	    //Shell Open/Close Status (0=Closed, 1=Is/was Open)
			uint8_t read : 1;			//Read in Progress (0=Not Reading, 1=Reading)
			uint8_t seek : 1;		    //Seek in Progress (0=Not Seeking, 1=Seeking)	
			uint8_t play : 1;		    //Play in Progress (0=Not Playing, 1=Playing)
		};
		
	};

	union StatusRegister
	{
		uint8_t		byte;

		struct
		{
			uint8_t index : 2;			//Current Register Bank ([R/W] 0..3 = Index0..Index3)
			uint8_t adpbusy : 1;		//ADPCM busy            ([R]   0=not playing XA-ADPCM, 1=playing XA-ADPCM)
			uint8_t prmempt : 1;		//Parameter empty       ([R]   0=parameter present in the FIFO, 1=parameter FIFO empty
			uint8_t prmwrdy : 1;		//Parameter write ready ([R]   0=parameter FIFO full, 1=parameter FIFO can be written
			uint8_t rslrrdy : 1;		//Result read ready     ([R]   0=result FIFO Empty, 1=result FIFO contains data
			uint8_t drqsts : 1;			//Data request          ([R]   0=no data request pending, 1=data request pending
			uint8_t busysts : 1;		//Busy status           ([R]   0=controller is idle, 1=controller is busy)
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
			uint8_t	cdda : 1;			//(0=Off, 1=Allow to Read CD-DA Sectors; ignore missing EDC)
			uint8_t autopause : 1;		//(0=Off, 1=Auto Pause upon End of Track) ;for Audio Play
			uint8_t	report : 1;			//(0=Off, 1=Enable Report-Interrupts for Audio Play)
			uint8_t xa_filter : 1;		//(0=Off, 1=Process only XA-ADPCM sectors that match Setfilter)
			uint8_t ignore_bit : 1;     //(0=Normal, 1=Ignore Sector Size and Setloc position)
			uint8_t sector_size : 1;    //(0=800h=DataOnly, 1=924h=WholeSectorExceptSyncBytes)
			uint8_t xa_adpcm : 1;		//(0=Off, 1=Send XA-ADPCM sectors to SPU Audio Input)
			uint8_t speed : 1;			//(0=Normal speed, 1=Double speed)
		};
	};

	union InterruptMaskRegister
	{
		uint8_t byte;

		struct
		{
			uint8_t enint : 3;			//Enable IRQ on respective INTSTS bit (INT1, INT2, INT3, ...)
			uint8_t enbfempt : 1;		//Enable IRQ on BFREMPT status change
			uint8_t enbfwrdy : 1;   	//Enable IRQ on BFWRDY status change
			uint8_t unused : 3;			//Reserved. Always 0x0 when written, always 0x7 when read
		};
	};

	union InterruptStatusRegister
	{
		uint8_t byte;

		struct
		{
			uint8_t intsts : 3;			//INTSTS Interrupt "flags" from HC050 (INT1, INT2, INT3, ...)
			uint8_t bfempt : 1;			//BFEMPT Sound map XA-ADPCM buffer empty
			uint8_t bfwrdy : 1;			//BFWRDY Sound map XA-ADPCM buffer write ready
			uint8_t unused : 3;			//Reserved. Always 0x7 when read and write.
		};
	};

	enum {INT1 = 1, INT2 = 2, INT3 = 3, INT5 = 5};
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
	
	//Connect to PSX Instance
	void link(Psx* instance) { psx = instance; }

    //Getter & Setters
    uint8_t getStatusRegister() const { return statusRegister.byte; }
	uint8_t getRequestRegister() const { return requestRegister.byte; }
	uint8_t getInterruptMaskRegister() const { return interruptMaskRegister.byte; }
	uint8_t getInterruptStatusRegister() const { return interruptStatusRegister.byte; }	

private:
	//Link to Bus Object
	Psx* psx;
	//CD Image Object
	CdImage	cdImage;

	//Internal Status Flags
	bool cdImageLoaded;
	bool cdShellOpen;
	bool cdMotorOn;
	bool isStreamingData;
	uint32_t streamingINT1Delay;

	//Internal Registers
	cdrom::StatusRegister			statusRegister;
	cdrom::RequestRegister			requestRegister;
	cdrom::ModeRegister				modeRegister;
	cdrom::InterruptMaskRegister	interruptMaskRegister;
	cdrom::InterruptStatusRegister	interruptStatusRegister;

	//Internal Fifo
	lite::fifo<uint8_t, 16> 				commandFifo;   //Not really needed, used to check if multiple commands are sent before execution
	lite::fifo<uint8_t, 16> 				parameterFifo;
	lite::fifo<uint8_t, 2048 * 2> 			dataFifo;
	lite::fifo<uint8_t, 16> 				responseFifo;
	lite::fifo<cdrom::InterruptEvent, 16> 	interruptFifo;
	lite::fifo<uint8_t, 2048 * 2> 			adpcmFifo;

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

