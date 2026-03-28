#include <loguru.hpp>
#include "spu.h"

SPU::SPU()
{
    //Reset Internal Registers
	mainVolumeL = 0x0;
	mainVolumeR = 0x0;
	reverbVolumeL = 0x0;
	reverbVolumeR = 0x0;
	voiceKeyOn = 0x0;
	voiceKeyOff = 0x0;
	voiceChannelFM = 0x0;
	voiceChannelNoise = 0x0;
	voiceChannelReverb = 0x0;
	voiceChannelStatus = 0x0;
	unknownReg1 = 0x0;
	soundRamAddr = 0x0;
	soundRamIrqAddr = 0x0;
	soundRamDataTransAddr = 0x0;
	soundRamDataTransFifo = 0x0;
	spuCnt = 0x0;
	soundRamDataTransCtrl = 0x0;
	spuStat = 0x0;
	cdromVolumeL = 0x0;
	cdromVolumeR = 0x0;
	externVolumeL = 0x0;
	externVolumeR = 0x0;
	currentVolumeL = 0x0;
	currentVolumeR = 0x0;
	unknownReg2 = 0x0;
}

SPU::~SPU()
{
}

bool SPU::writeAddr(uint32_t addr, uint32_t& data, uint8_t bytes)
{
	switch (addr)
	{
	case 0x1f801d80:
		mainVolumeL = (uint16_t)data;			//Main Volume Left
		break;
	case 0x1f801d82:
		mainVolumeR = (uint16_t)data;			//Main Volume Right
		break;
	case 0x1f801d84:
		reverbVolumeL = (uint16_t)data;			//Reverb Output Volume Left
		break;
	case 0x1f801d86:
		reverbVolumeR = (uint16_t)data;			//Reverb Output Volume Right
		break;
	case 0x1f801d88:
		voiceKeyOn = data;						//Voice 0..23 Key ON (Start Attack/Decay/Sustain) (W)
		break;
	case 0x1f801d8c:
		voiceKeyOff = data;						//Voice 0..23 Key OFF (Start Release) (W)
		break;
	case 0x1f801d90:
		voiceChannelFM = data;					//Voice 0..23 Channel FM (pitch lfo) mode (R/W)
		break;
	case 0x1f801d94:
		voiceChannelNoise = data;				//Voice 0..23 Channel Noise mode (R/W)
		break;
	case 0x1f801d98:
		voiceChannelReverb = data;				//Voice 0..23 Channel Reverb mode (R/W)
		break;
	case 0x1f801d9c:
		voiceChannelStatus = data;				//Voice 0..23 Channel ON/OFF (status) (R)
		break;
	case 0x1f801da0:
		unknownReg1 = (uint16_t)data;			//Unknown
		break;
	case 0x1f801da2:
		soundRamAddr = (uint16_t)data;			//Sound RAM Reverb Work Area Start Address
		break;
	case 0x1f801da4:
		soundRamIrqAddr = (uint16_t)data;		//Sound RAM IRQ Address
		break;
	case 0x1f801da6:
		soundRamDataTransAddr = (uint16_t)data; //Sound RAM Data Transfer Address
		break;
	case 0x1f801da8:
		soundRamDataTransFifo = (uint16_t)data;	//Sound RAM Data Transfer Fifo
		break;
	case 0x1f801daa:
		spuCnt = (uint16_t)data;				//SPU Control Register (SPUCNT)
		break;
	case 0x1f801dac:
		soundRamDataTransCtrl = (uint16_t)data; //Sound RAM Data Transfer Control
		break;
	case 0x1f801dae:
		spuStat = (uint16_t)data;				//SPU Status Register (SPUSTAT) (R)
		break;
	case 0x1f801db0:
		cdromVolumeL = (uint16_t)data;			//CD Volume Left
		break;
	case 0x1f801db2:
		cdromVolumeR = (uint16_t)data;			//CD Volume Right
		break;
	case 0x1f801db4:
		externVolumeL = (uint16_t)data;			//Extern Volume Left
		break;
	case 0x1f801db6:
		externVolumeR = (uint16_t)data;			//Extern Volume Right
		break;
	case 0x1f801db8:
		currentVolumeL = (uint16_t)data;		//Current Main Volume Left
		break;
	case 0x1f801dba:
		currentVolumeR = (uint16_t)data;		//Current Main Volume Right
		break;
	case 0x1f801dbc:
		unknownReg2 = data;						//Unknown
		break;
	
	default:
		//LOG_F(ERROR, "SPU - Write Unknown Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
		return false;
	}

	LOG_F(3, "SPU - Write to Register:\t\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
	return true;
}

uint32_t SPU::readAddr(uint32_t addr, uint8_t bytes)
{
	uint32_t data = 0x0;

	switch (addr)
	{
	case 0x1f801d80:
		data = mainVolumeL;						//Main Volume Left
		break;
	case 0x1f801d82:
		data = mainVolumeR;						//Main Volume Right
		break;
	case 0x1f801d84:
		data = reverbVolumeL;					//Reverb Output Volume Left
		break;
	case 0x1f801d86:
		data = reverbVolumeR;					//Reverb Output Volume Right
		break;
	case 0x1f801d88:
		data = voiceKeyOn;						//Voice 0..23 Key ON (Start Attack/Decay/Sustain) (W)
		break;
	case 0x1f801d8c:
		data = voiceKeyOff;						//Voice 0..23 Key OFF (Start Release) (W)
		break;
	case 0x1f801d90:
		data = voiceChannelFM;					//Voice 0..23 Channel FM (pitch lfo) mode (R/W)
		break;
	case 0x1f801d94:
		data = voiceChannelNoise;				//Voice 0..23 Channel Noise mode (R/W)
		break;
	case 0x1f801d98:
		data = voiceChannelReverb;				//Voice 0..23 Channel Reverb mode (R/W)
		break;
	case 0x1f801d9c:
		data = voiceChannelStatus;				//Voice 0..23 Channel ON/OFF (status) (R)
		break;
	case 0x1f801da0:
		data = unknownReg1;						//Unknown
		break;
	case 0x1f801da2:
		data = soundRamAddr;					//Sound RAM Reverb Work Area Start Address
		break;
	case 0x1f801da4:
		data = soundRamIrqAddr;					//Sound RAM IRQ Address
		break;
	case 0x1f801da6:
		data = soundRamDataTransAddr;			 //Sound RAM Data Transfer Address
		break;
	case 0x1f801da8:
		data = soundRamDataTransFifo;			//Sound RAM Data Transfer Fifo
		break;
	case 0x1f801daa:
		data = spuCnt;							//SPU Control Register (SPUCNT)
		break;
	case 0x1f801dac:
		data = soundRamDataTransCtrl;			//Sound RAM Data Transfer Control
		break;
	case 0x1f801dae:
		data = spuStat;							//SPU Status Register (SPUSTAT) (R)
		break;
	case 0x1f801db0:
		data = cdromVolumeL;					//CD Volume Left
		break;
	case 0x1f801db2:
		data = cdromVolumeR;					//CD Volume Right
		break;
	case 0x1f801db4:
		data = externVolumeL;					//Extern Volume Left
		break;
	case 0x1f801db6:
		data = externVolumeR;					//Extern Volume Right
		break;
	case 0x1f801db8:
		data = currentVolumeL;					//Current Main Volume Left
		break;
	case 0x1f801dba:
		data = currentVolumeR;					//Current Main Volume Right
		break;
	case 0x1f801dbc:
		data = unknownReg2;						//Unknown
		break;

	default:
		//LOG_F(ERROR, "SPU - Read Unknown Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
		return 0x0;
	}

	LOG_F(3, "SPU - Read from Register:\t\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
	return data;
}
