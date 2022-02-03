#pragma once

#include <cstdint>
#include <cstdio>

class Psx;

class SPU
{
public:
	SPU();
	~SPU();

	bool setParameter(uint32_t addr, uint32_t& data, uint8_t bytes);
	uint32_t getParameter(uint32_t addr, uint8_t bytes);

	//Connect to PSX Instance
	void link(Psx* instance) { psx = instance; }

private:
	//Link to Bus Object
	Psx* psx = nullptr;

	//Internal Registers
	uint16_t mainVolumeL;			//0x1f801d80
	uint16_t mainVolumeR;			//0x1f801d82
	uint32_t reverbVolumeL;			//0x1f801d84
	uint32_t reverbVolumeR;			//0x1f801d86
	uint32_t voiceKeyOn;			//0x1f801d88
	uint32_t voiceKeyOff;			//0x1f801d8c
	uint32_t voiceChannelFM;		//0x1f801d90
	uint32_t voiceChannelNoise;		//0x1f801d94
	uint32_t voiceChannelReverb;	//0x1f801d98
	uint32_t voiceChannelStatus;	//0x1f801d9c
	uint16_t unknownReg1;			//0x1f801da0
	uint16_t soundRamAddr;			//0x1f801da2
	uint16_t soundRamIrqAddr;		//0x1f801da4
	uint16_t soundRamDataTransAddr;	//0x1f801da6
	uint16_t soundRamDataTransFifo;	//0x1f801da8
	uint16_t spuCnt;				//0x1f801daa
	uint16_t soundRamDataTransCtrl;	//0x1f801dac
	uint16_t spuStat;				//0x1f801dae
	uint16_t cdromVolumeL;			//0x1f801db0
	uint16_t cdromVolumeR;			//0x1f801db2
	uint16_t externVolumeL;			//0x1f801db4
	uint16_t externVolumeR;			//0x1f801db6
	uint16_t currentVolumeL;		//0x1f801db8
	uint16_t currentVolumeR;		//0x1f801dba
	uint32_t unknownReg2;			//0x1f801dbc
};

