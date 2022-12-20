#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>
#include <string>
#include <memory>

#include "renderer.h"
#include "utils.h"
#include "timers.h"
#include "litelib.h"

class Psx;

//PAL Magic Numbers
constexpr auto PAL_VISIBLE_SCANLINES = 288;
constexpr auto PAL_VBLANK_SCANLINES = 25+1;
constexpr auto PAL_HBLANK_GPU_TICK = 638;
constexpr auto PAL_ACTIVE_GPU_TICK = 2766;

//NTSC Magic Numbers
constexpr auto NTSC_VISIBLE_SCANLINES = 240;
constexpr auto NTSC_VBLANK_SCANLINES = 23+1;
constexpr auto NTSC_HBLANK_GPU_TICK = 585;
constexpr auto NTSC_ACTIVE_GPU_TICK = 2827;

//GPU Constants
constexpr auto VRAM_SIZE = 1024 * 512;
constexpr auto MAX_POLYGON_PARAMS = 3;

enum class VideoMode { NTSC = 0x0, PAL = 0x1 };

//GPU Debug Status
struct GpuDebugInfo
{
	uint32_t					gpuStat;
	void*						vRam;
	lite::vec2t<uint16_t>		displayStart;
	lite::vec4t<uint16_t>		displayRange;
	lite::vec2t<uint16_t>		drawingOffset;
	lite::vec4t<uint16_t>		drawingArea;
	lite::vec2t<uint16_t>		videoResolution;
	std::string					videoStandard;
	std::string					textureAllowDisable;
	std::string					textureDisable;
	lite::vec2t<uint16_t>		texturePage;
	std::string					texturePageColor;
	lite::vec2t<uint8_t>		textureMask;
	lite::vec2t<uint8_t>		textureOffset;
};

union GPUSTAT
{
	uint32_t data;

	struct
	{
		uint8_t texPageX : 4;			//GPUSTAT.0-3
		uint8_t texPageY : 1;			//GPUSTAT.4
		uint8_t semiTransparency : 2;	//GPUSTAT.5-6
		uint8_t texPageColors : 2;		//GPUSTAT.7-8
		uint8_t ditherMode : 1;			//GPUSTAT.9
		uint8_t drawEnable : 1;			//GPUSTAT.10
		uint8_t maskPixel : 1;			//GPUSTAT.11
		uint8_t drawPixel : 1;			//GPUSTAT.12
		uint8_t interlaceField : 1;		//GPUSTAT.13
		uint8_t reverseFlag : 1;		//GPUSTAT.14
		uint8_t texDisable : 1;			//GPUSTAT.15
		uint8_t hRes2 : 1;				//GPUSTAT.16
		uint8_t hRes1 : 2;				//GPUSTAT.17-18
		uint8_t vRes : 1;				//GPUSTAT.19
		uint8_t videoMode : 1;			//GPUSTAT.20
		uint8_t displayColor : 1;		//GPUSTAT.21
		uint8_t vInterlace : 1;			//GPUSTAT.22
		uint8_t displayEnable : 1;		//GPUSTAT.23 
		uint8_t interruptReq : 1;		//GPUSTAT.24
		uint8_t dmaDataReq : 1;			//GPUSTAT.25
		uint8_t readyRecvCmdWord : 1;	//GPUSTAT.26
		uint8_t readySendVRAM2CPU : 1;	//GPUSTAT.27
		uint8_t readyRecvDmaBlock : 1;	//GPUSTAT.28
		uint8_t dmaDirection : 2;		//GPUSTAT.29-30
		uint8_t drawingOddLine : 1;		//GPUSTAT.31
	};
};

class GPU
{
public:
	GPU();
	~GPU();

	bool reset();
	bool clock();

	bool writeAddr(uint32_t addr, uint32_t& data, uint8_t bytes = 4);
	uint32_t readAddr(uint32_t addr, uint8_t bytes = 4);

	bool isFrameReady();

	//Connect to PSX Instance
	void link(Psx* instance) { psx = instance; }

	//Debug Info
	void getDebugInfo(GpuDebugInfo& info);

public:
	bool hBlank;
	bool vBlank;

	//OpenGL Renderer
	std::shared_ptr<Renderer>	pRenderer;

private:
	void writeVRAM(uint32_t& data);
	uint32_t readVRAM();

	//Helper Function
	void DrawVRAMRectangle();


private:
	//Link to Bus Object
	Psx* psx;

	//VRAM 1MB
	uint16_t	vRam[512][1024];	//vram[rows][pixels]
	
	//Internal Registers
	uint32_t	gp0DataLatch;
	uint32_t	gp1DataLatch;
	uint32_t	gpuReadLatch;
	uint32_t	gpuStat;
	
	//Command FIFO
	lite::fifo<uint32_t, 16>	fifo;

	//Vertex Info
	std::vector<VertexInfo> vertexPolyInfo;
	std::vector<VertexInfo> vertexRectInfo;
	std::vector<VertexInfo> vertexLineInfo;

	//GPU Internal Status & Configurations
	VideoMode			videoMode;
	uint8_t				dotClockRatio;
	bool				verticalInterlace;
	lite::vec2t<uint16_t>		videoResolution;
	lite::vec2t<uint16_t>		displayStart;
	lite::vec4t<uint16_t>		displayRange;
	lite::vec2t<uint16_t>		drawingOffset;
	lite::vec4t<uint16_t>		drawingArea;
	lite::vec2t<uint8_t>		textureMask;
	lite::vec2t<uint8_t>		textureOffset;
	lite::vec2t<uint16_t>		texturePage;
	uint8_t				texturePageColor;

	bool				textureAllowDisable;
	bool				textureDisable;
	
	uint8_t				dmaDirection;			//Set by GP1(04h), Contains actual value for GPUSTAT.29-30 (0 = off, 1 = FIFO, 2 =  RAM to VRAM, 3 = VRAM to RAM)
	bool				recvCommand;			//True if a GP0/GP1 command has been received
	bool				recvParameters;			//True if all GP0 Parameters has been received or if current OpCode has zero parameters 
	bool				gp0CommandAvailable;	//A full GP0 command is available with all parameters
	uint8_t				gp0Opcode;				//Current Available GP0 Opcode
	uint32_t			gp0Command;				//Current Available GP0 Command
	uint8_t				gp0CommandParameters;	//Number of parameters for the current GP0 Command (255 is a special value)
	uint8_t				gp0ReadParameters;		//Used as a counter while receiving parameter for the current gp0Commnad
	bool				gp0CommandFifo;			//True if current GP0 command is on the FIFO
	bool				gp0RecvPolyLine;		//True if current GP0 command is for a Polygon or a Line 
	bool				gp1CommandAvailable;	//A full GP1 command is available
	uint8_t				gp1Opcode;				//Current available GP1 Opcode
	uint32_t			gp1Command;				//Current Available GP1 Command

	uint32_t			hCount;				//Count GPU Ticks per Scanline
	uint32_t			vCount;				//Count the number of Scanlines
	bool				newScanline;		//Set if a new Scanline has startes
	bool				newFrameReady;		//Set if a new Frame has started
		
	//Memory Transfer Status
	lite::vec2t<uint16_t>		dataDestination;	//Framebuffer destination start point: x is offset in halfwords, y is offset in rows
	lite::vec2t<uint16_t>		dataSource;			//Framebuffer source start point: x is offset in halfwords, y is offset in rows
	lite::vec2t<uint16_t>		dataSize;			//Data Rectangle size: x is in halfwords, y is in rows
	lite::vec2t<uint16_t>		dataPointer;		//Pointer to current read or write address in VRAM
	bool						dataReadActive;		//Set by GP0(C0h), enable data tranfer from VRAM to RAM 
	bool						dataWriteActive;	//Set by GP0(A0h), enable data transfer from RAM to VRAM
		
	//Internal Clock Counter
	uint64_t	gpuClockTicks;
	
	//Full set GPU Instruction Dictionaries
	struct INSTRGP0
	{
		std::string mnemonic;
		bool(GPU::* operate)() = nullptr;
		uint8_t parameters;
		bool fifo;
		bool polyline;
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
	bool gp0_FillVRam();
	bool gp0_InterruptRequest();
	bool gp0_Polygons();
	bool gp0_Lines();
	bool gp0_Rectangles();
	bool gp0_CopyVRam2VRam();
	bool gp0_CopyRam2VRam();
	bool gp0_CopyVRam2Ram();
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

