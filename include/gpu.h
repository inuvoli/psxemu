#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>
#include <string>

#include "commandfifo.h"

class Psx;

//PAL Magic Numbers
constexpr auto PAL_VISIBLE_SCANLINES = 288;
constexpr auto PAL_VBLANK_SCANLINES = 25;
constexpr auto PAL_HBLANK_GPU_TICK = 638;
constexpr auto PAL_ACTIVE_GPU_TICK = 2766;

//NTSC Magic Numbers
constexpr auto NTSC_VISIBLE_SCANLINES = 240;
constexpr auto NTSC_VBLANK_SCANLINES = 23;
constexpr auto NTSC_HBLANK_GPU_TICK = 585;
constexpr auto NTSC_ACTIVE_GPU_TICK = 2827;

//GPU Constants
constexpr auto VRAM_SIZE = 1024 * 512;

enum class VideoMode { NTSC = 0x0, PAL = 0x1 };

template <typename T>
struct Pair
{
	T	x;
	T	y;

	/*Pair(const T x, const T y)
	{
		this->x = x;
		this->y = y;
	}*/

	Pair& operator= (const Pair& t)
	{
		x = t.x;
		y = t.y;
		return *this;
	}
};

template <typename T>
struct Quad
{
	T	x1;
	T	y1;
	T	x2;
	T	y2;

	Quad& operator= (const Quad& t)
	{
		x1 = t.x1;
		y1 = t.y1;
		x2 = t.x2;
		y2 = t.y2;
		return *this;
	}
};

//GPU Debug Status
struct GpuDebugInfo
{
	uint32_t			gpuStat;
	void*				vRam;
	Pair<uint16_t>		displayOffset;
	Quad<uint16_t>		displayArea;
	Pair<uint16_t>		drawingOffset;
	Quad<uint16_t>		drawingArea;
	Pair<uint16_t>		videoResolution;
	std::string			videoStandard;
};

class GPU
{
public:
	GPU();
	~GPU();

	bool reset();
	bool clock();

	bool setParameter(uint32_t addr, uint32_t& data, uint8_t bytes = 4);
	uint32_t getParameter(uint32_t addr, uint8_t bytes = 4);

	bool isFrameReady();

	//Connect to PSX Instance
	void link(Psx* instance) { psx = instance; }

	//Debug Info
	void getDebugInfo(GpuDebugInfo& info);

public:
	bool hBlank;
	bool vBlank;

private:
	void writeVRAM(uint32_t& data);
	uint32_t readVRAM();

private:
	//Link to Bus Object
	Psx* psx = nullptr;

	//VRAM 1MB
	uint16_t			vRam[512][1024];	//vram[rows][pixels]

	//Internal Registers
	uint32_t	gp0DataLatch;
	uint32_t	gp1DataLatch;
	uint32_t	gpuReadLatch;
	uint32_t	gpuStat;
	
	//Command FIFO
	CommandFifo<uint32_t, 16>	fifo;

	//GPU Internal Status & Configurations
	VideoMode			videoMode;
	uint16_t			horizontalResolution1;
	uint16_t			horizontalResolution2;
	uint8_t				dotClockRatio;
	uint16_t			verticalResolution;
	bool				verticalInterlace;
	Pair<uint16_t>		displayOffset;
	Quad<uint16_t>		displayArea;
	Pair<uint16_t>		drawingOffset;
	Quad<uint16_t>		drawingArea;
	Pair<uint8_t>		textureMask;
	Pair<uint8_t>		textureOffset;

	uint8_t				dmaDirection;
	bool				recvCommand;
	bool				recvParameters;
	bool				gp0CommandAvailable;
	uint32_t			gp0Command;
	uint8_t				gp0CommandParameters;
	bool				gp0CommandFifo;
	bool				gp1CommandAvailable;
	uint32_t			gp1Command;

	uint32_t			hCount;				//Count GPU Ticks per Scanline
	uint32_t			vCount;				//Count the number of Scanlines
	bool				newScanline;		//Set if a new Scanline has startes
	bool				newFrameReady;		//Set if a new Frame has started
		
	//Memory Transfer Status
	Pair<uint16_t>		dataDestination;	//Framebuffer destination start point: x is offset in halfwords, y is offset in rows
	Pair<uint16_t>		dataSource;			//Framebuffer source start point: x is offset in halfwords, y is offset in rows
	Pair<uint16_t>		dataSize;			//Data Rectangle size: x is in halfwords, y is in rows
	Pair<uint16_t>		dataPointer;		//Pointer to current read or write address in VRAM
	bool				dataReadActive;		//Set by GP0(C0h), enable data tranfer from VRAM to RAM 
	bool				dataWriteActive;	//Set by GP0(A0h), enable data transfer from RAM to VRAM
		
	//Internal Clock Counter
	uint64_t	gpuClockTicks;

	//Full set GPU Instruction Dictionaries
	struct INSTRGP0
	{
		std::string mnemonic;
		bool(GPU::* operate)() = nullptr;
		uint8_t parameters;
		bool fifo;
	};

	struct INSTRGP1
	{
		std::string mnemonic;
		bool(GPU::* operate)() = nullptr;
	};

	std::vector<INSTRGP0> gp0InstrSet;		//Full GP0 Instruction Set
	std::vector<INSTRGP1> gp1InstrSet;		//Full GP1 Instruction Set

	//GP0 Instructions
	bool gp0_NoOperation();
	bool gp0_ClearTextureCache();
	bool gp0_ClearScreen();
	bool gp0_InterruptRequest();
	bool gp0_Polygons();
	bool gp0_Lines();
	bool gp0_Rectangles();
	bool gp0_Vram2Vram();
	bool gp0_Ram2Vram();
	bool gp0_Vram2Ram();
	bool gp0_DrawMode();
	bool gp0_TextureSetting();
	bool gp0_SetDrawAreaTop();
	bool gp0_SetDrawAreaBottom();
	bool gp0_SetDrawOffset();
	bool gp0_SetMaskBit();
	
	void gp0_ResetStatus();

	//GP1 Instructions
	bool gp1_ResetGpu();
	bool gp1_ResetFifo();
	bool gp1_AckInterrupt();
	bool gp1_DisplayEnable();
	bool gp1_DmaDirection();
	bool gp1_StartDisplayArea();
	bool gp1_HDisplayRange();
	bool gp1_VDisplayRange();
	bool gp1_DisplayMode();
	bool gp1_TextureDisable();
	bool gp1_Unknown();
	bool gp1_GetGpuInfo();

	void gp1_ResetStatus();

};

