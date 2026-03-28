#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>
#include <string>
#include <memory>

#include "litelib.h"
#include "gpu_utils.h"

class Psx;

//NTSC Magic Numbers
constexpr auto NTSC_GPU_CLOCK_PER_SCANLINE = 3413;
constexpr auto NTSC_GPU_CLOCK_PER_HBLANK = 580;
constexpr auto NTSC_SCANLINES_PER_FRAME = 263;
constexpr auto NTSC_SCANLINES_PER_VBLANK = 23;

//PAL Magic Numbers
constexpr auto PAL_GPU_CLOCK_PER_SCANLINE = 3406;
constexpr auto PAL_GPU_CLOCK_PER_HBLANK = 638;
constexpr auto PAL_SCANLINES_PER_FRAME = 314;
constexpr auto PAL_SCANLINES_PER_VBLANK = 25;

//GPU Constants
constexpr auto VRAM_SIZE = 1024 * 512;
constexpr auto MAX_POLYGON_PARAMS = 3;
constexpr auto MAX_RECTANGLE_PARAMS = 4;

//GPU Video Modes
enum class VideoMode { NTSC = 0x0, PAL = 0x1 };

//GPU Max Vertex Number per Primitive
constexpr auto MAX_VERTEX_NUMBER = 4;

//GPUSTAT Register Structure
namespace gpu
{
	union StatusRegister
	{
		uint32_t word;

		struct
		{
			lite::bitfield<0, 4>   texPageX;				//Texture page X Base       (N*64)                                   ;GP0(E1h).0-3
			lite::bitfield<4, 1>   texPageY;				//Texture page Y Base       (N*256) (ie. 0 or 256)                   ;GP0(E1h).4
			lite::bitfield<5, 2>   semiTransMode;			//Semi Transparency Mode    (0=B/2+F/2, 1=B+F, 2=B-F, 3=B+F/4)       ;GP0(E1h).5-6
			lite::bitfield<7, 2>   texColorMode;     		//Texture page colors       (0=4bit, 1=8bit, 2=15bit, 3=Reserved)    ;GP0(E1h).7-8
			lite::bitfield<9, 1>   dither;		            //Dither 24bit to 15bit     (0=Off/strip LSBs, 1=Dither Enabled)     ;GP0(E1h).9
			lite::bitfield<10, 1>  drawEnable;				//Drawing to Drawing Area   (0=Prohibited, 1=Allowed)                ;GP0(E1h).10
			lite::bitfield<11, 1>  forceMask;				//Force Mask-bit when drawing pixels (0=No, 1=Yes/Mask)              ;GP0(E6h).0
			lite::bitfield<12, 1>  checkMask;				//Check Mask drawing Pixels (0=Always, 1=Not to Masked areas)        ;GP0(E6h).1
			lite::bitfield<13, 1>  interlaceField;			//Interlace Field           (or, always 1 when GP1(08h).5=0)
			lite::bitfield<14, 1>  reverseFlag;				//"Reverseflag"             (0=Normal, 1=Distorted)                  ;GP1(08h).7			
			lite::bitfield<15, 1>  texDisable;				//Texture Disable           (0=Normal, 1=Disable Textures)           ;GP0(E1h).11
			lite::bitfield<16, 1>  hRes2;					//Horizontal Resolution 2   (0=256/320/512/640, 1=368)               ;GP1(08h).6
			lite::bitfield<17, 2>  hRes1;					//Horizontal Resolution 1   (0=256, 1=320, 2=512, 3=640)             ;GP1(08h).0-1
			lite::bitfield<19, 1>  vRes;					//Vertical Resolution       (0=240, 1=480, when Bit22=1)             ;GP1(08h).2
			lite::bitfield<20, 1>  videoMode;				//Video Mode                (0=NTSC/60Hz, 1=PAL/50Hz)                ;GP1(08h).3
			lite::bitfield<21, 1>  displayColorMode;		//Display Area Color Depth  (0=15bit, 1=24bit)                       ;GP1(08h).4
			lite::bitfield<22, 1>  vertInterlace;			//Vertical Interlace        (0=Off, 1=On)                            ;GP1(08h).5
			lite::bitfield<23, 1>  displayEnabled;			//Display Enable            (0=Enabled, 1=Disabled)                  ;GP1(03h).0
			lite::bitfield<24, 1>  interruptReq;			//Interrupt Request (IRQ1)  (0=Off, 1=IRQ)                           ;GP0(1Fh)/GP1(02h)
			lite::bitfield<25, 1>  dmaDataReq;				//DMA / Data Request, meaning depends on GP1(04h) DMA Direction
			lite::bitfield<26, 1>  readyRecvCmdWord;		//Ready to receive Cmd Word (0=No, 1=Ready)                          ;GP0(...) ;via GP0
			lite::bitfield<27, 1>  readySendVRAM2CPU;		//Ready to send VRAM to CPU (0=No, 1=Ready)                          ;GP0(C0h) ;via GPUREAD
			lite::bitfield<28, 1>  readyRecvDMABlock;		//Ready to receive DMA Block(0=No, 1=Ready)                          ;GP0(...) ;via GP0
			lite::bitfield<29, 2>  dmaDirection;			//DMA Direction             (0=Off, 1=?, 2=CPUtoGP0, 3=GPUREADtoCPU) ;GP1(04h).0-1
			lite::bitfield<31, 1>  drawingOddLine;			//Drawing even/odd lines in interlace mode (0=Even or Vblank, 1=Odd)
		};
	};

	struct VideoMemoryAccessState
	{
		bool				  write;						//Whether GPU is currently writing to VRAM
		bool				  read;							//Whether GPU is currently reading from VRAM	
		lite::vec2t<uint16_t> src;							//VRAM Coordinates where start reading
		lite::vec2t<uint16_t> dst;							//VRAM Coordinates where start writing
		lite::vec2t<uint16_t> size;							//Rectangle in VRAM to read/write
		lite::vec2t<uint16_t> writePos;						//Current VRAM Coordinates to write to, auto-incremented by GPU when writing to VRAM.
		lite::vec2t<uint16_t> readPos;						//Current VRAM Coordinates to read from, auto-incremented by GPU when reading from VRAM.
		uint32_t			  dataWrite;					//Number of halfords written to VRAM, auto-incremented by GPU when writing to VRAM.
		uint32_t			  dataRead;						//Number of halfords read from VRAM, auto-incremented by GPU when reading from VRAM.
		uint32_t			  dataLenght;					//Total number of halfords to read/write in current VRAM transfer operation, set by GP0 command parameters.
	};

	struct CommandPipelineState
	{
		bool					commandReceived;			//Whether GPU is currently receiving a command and its parameters
		bool					parametersReceived;			//Whether GPU has received all parameters for current command and is ready to execute it
		bool					isPolyline;					//Whether current command is a Polyline command, which has special handling for vertex parameters
	};

}

//GPU Class
class GPU
{
public:
	GPU();
	~GPU();

	bool reset();
	bool execute();
	bool runticks();

	bool writeAddr(uint32_t addr, uint32_t& data, uint8_t bytes = 4);
	uint32_t readAddr(uint32_t addr, uint8_t bytes = 4);

	//Connect to PSX Instance
	void link(Psx* instance) { psx = instance; }

	//Getter for GPU Internal Registers
	uint32_t getGPUStat() const { return gpuStat; }
	
	//Getter for Display Area Status & Configurations
	lite::vec2t<uint16_t> getDisplayStart() const { return displayStart; }
	lite::vec4t<uint16_t> getDisplayRange() const { return displayRange; }
	lite::vec2t<uint16_t> getDisplayResolution() const { return displayResolution; }
	std::string getDisplayMode() const { return (displayMode == VideoMode::NTSC) ? "NTSC" : "PAL"; }
	std::string getDisplayColorMode() const { return (displayColorMode == 0) ? "15bit" : "24bit"; }
	std::string getDisplayDisabled() const { return displayDisabled ? "True" : "False"; }
	std::string getVerticalInterlace() const { return verticalInterlace ? "Interlaced" : "Progressive"; }
		
	//Getter for Drawing Area Status & Configurations
	lite::vec4t<uint16_t> getDrawingArea() const { return drawingArea; }
	lite::vec2t<uint16_t> getDrawingOffset() const { return drawingOffset; }
	lite::vec2t<uint8_t> getTextureMask() const { return textureMask; }
	lite::vec2t<uint8_t> getTextureOffset() const { return textureOffset; }
	lite::vec2t<uint16_t> getTexturePage() const { return texturePage; }
	std::string getSemiTransparencyMode() const 
	{ 
		switch (semiTransparencyMode)
		{
			case 0: return "B/2+F/2";
			case 1: return "B+F";
			case 2: return "B-F";
			case 3: return "B+F/4";
			default: return "Reserved";
		}
	}
	std::string getTextureColorDepth() const
	{
		switch (colorMode)
		{
		case 0: return "4bit CLUT";
		case 1: return "8bit CLUT";
		case 2: return "15bit ABGR (1.5.5.5)";
		default: return "Reserved";
		}
	}
	std::string getDitherEnabled() const { return ditherEnabled ? "True" : "False"; }
	std::string getDrawingOnDisplayEnabled() const { return drawingOnDisplayEnabled ? "True" : "False"; }
	std::string getRectangleTexFlipX() const { return rectangleTexFlipX ? "True" : "False"; }
	std::string getRectangleTexFlipY() const { return rectangleTexFlipY ? "True" : "False"; }
	std::string getTextureDisabled() const { return textureDisabled ? "True" : "False"; }
	std::string getForceMask() const { return forceMask ? "True" : "False"; }
	std::string getCheckMask() const { return checkMask ? "True" : "False"; }

public:
	bool hBlank;
	bool vBlank;

private:
	bool writeVRAM(uint32_t data);
	uint32_t readVRAM();
	bool updateVHBlank();

private:
	//Link to Bus Object
	Psx* psx;

	
	
	//GPU Internal Registers
	uint32_t					gp0DataLatch;				//GP0 Command Latch
	uint32_t					gp1DataLatch;				//GP1 Commans Latch
	uint32_t					gpuReadLatch;				//GPUREAD
	uint32_t					gpuStat;					//GPUSTAT
	lite::fifo<uint32_t, 16>	fifo;						//Command Parameter FIFO	

	//GPU Video Display Status & Configurations
	lite::vec2t<uint16_t>		displayStart;				//Set by GP1(05h), Display Start Coordinates. (X,Y) in pixels				
	lite::vec4t<uint16_t>		displayRange;				//Set by GP1(06h) and GP1(07h), Display Range. (X1,Y1,X2,Y2) in pixels. Only pixels within this region will be displayed.
	lite::vec2t<uint16_t>		displayResolution;			//Set by GP1(08h), Display Resolution.							  ;GPUSTAT.17-18, .19, .16
	VideoMode					displayMode;				//Set by GP1(08h), Display Standard. (0=NTSC/60Hz, 1=PAL/50Hz)    ;GPUSTAT.20
	uint8_t						displayColorMode;			//Set by GP1(08h), Display Color Depth. (0=15bit, 1=24bit)        ;GPUSTAT.21			
	uint8_t						dotClockRatio;				//Set by GP1(08h), Dot Clock Ratio for Dot Timers. Set according to Horizontal Resolution
	bool						displayDisabled;			//Set by GP1(03h), (0 = Display On, 1 = Display Off)
	bool						verticalInterlace;			//Set by GP1(08h), Vertical Interlace. (0=Off, 1=On)              ;GPUSTAT.22
	
	//GPU Rendering Status & Configurations
	lite::vec4t<uint16_t>		drawingArea;				//Set by GP0(E3h) and GP0(E4h), Drawing Area. Rendering command clip any pixel outside this region.
	lite::vec2t<uint16_t>		drawingOffset;				//Set by GP0(E5h), Drawing Offset. offset in pixels steps applied to the Drawing Area.
	lite::vec2t<uint8_t>		textureMask;				//Set by GP0(E2h), Texture Window Mask (in 8 pixel steps). Texcoord = (Texcoord AND (NOT (Mask*8))) OR ((Offset AND Mask)*8)
	lite::vec2t<uint8_t>		textureOffset;				//Set by GP0(E2h), Texture Window Offset (in 8 pixel steps). Texcoord = (Texcoord AND (NOT (Mask*8))) OR ((Offset AND Mask)*8)
	lite::vec2t<uint16_t>		texturePage;				//Set by GP0(E1h), Texture Page X = N*64, Y = N*256
	uint8_t						semiTransparencyMode;		//Set by GP0(E1h), Transparency Mode (0=B/2+F/2, 1=B+F, 2=B-F, 3=B+F/4)
	uint8_t						colorMode;					//Set by GP0(E1h), Texture Page colors (0=4bit, 1=8bit, 2=15bit, 3=Reserved)
	bool						ditherEnabled;				//Set by GP0(E1h), Dither 24bit to 15bit (0=Off/strip LSBs, 1=Dither Enabled) 
	bool						drawingOnDisplayEnabled;	//Set by GP0(E1h), Drawing to display area (0=Prohibited, 1=Allowed)
	bool						rectangleTexFlipX;			//Set by GP0(E1h), Flip Texture Drawing in Textured Rectangles (0=normal, 1=flipped on X Axis)
	bool						rectangleTexFlipY;			//Set by GP0(E1h), Flip Texture Drawing in Textured Rectangles (0=normal, 1=flipped on Y Axis)
	bool						textureDisabled;			//Set by GP0(E1h), Texture Disable (0=Normal, 1=Disabled)
	bool						forceMask;					//Set by GP0(E6h), Force Mask Bit (0=Off, 1=On)
	bool						checkMask;					//Set by GP0(E6h), Check Mask Bit (0=Off, 1=On)
	
	//GPU Command Intepreter Status & Configuration
	bool						recvCommand;				//True if a GP0/GP1 command has been received
	bool						recvParameters;				//True if all GP0 Parameters has been received or if current OpCode has zero parameters 
	bool						gp0CommandAvailable;		//A full GP0 command is available with all parameters
	uint8_t						gp0Opcode;					//Current Available GP0 Opcode
	uint32_t					gp0Command;					//Current Available GP0 Command
	uint8_t						gp0CommandParameters;		//Number of parameters for the current GP0 Command (255 is a special value)
	uint8_t						gp0ReadParameters;			//Used as a counter while receiving parameter for the current gp0Commnad
	bool						gp0CommandFifo;				//True if current GP0 command is on the FIFO
	bool						gp0RecvPolyLine;			//True if current GP0 command is for a Polygon or a Line 
	bool						gp1CommandAvailable;		//A full GP1 command is available
	uint8_t						gp1Opcode;					//Current available GP1 Opcode
	uint32_t					gp1Command;					//Current Available GP1 Command
	gpu::CommandPipelineState	cmdPipelineState;			//GPU Command Pipeline State

	//GPU Timing Status & Configurations
	uint64_t					gpuClockTicks;				//GPU total Clock Tick counter
	uint32_t					hCount;						//Count GPU Ticks per Scanline
	uint32_t					vCount;						//Count the number of Scanlines
	uint32_t					tickCountPerScanline;		//The number of GPU Clock Ticks per Scanline, updated by GP1(08h) - Display Mode
	uint32_t					tickCountPerDots;			//The number of GPU Clock Ticks per Dots, updated by by GP1(08h) - Display Mode
	uint32_t					tickCountPerHBlank;			//The number of GPU Clock Ticks per HBlank, updated by by GP1(08h) - Display Mode
	uint32_t					scanlinePerFrame;			//The number of scanlines per frame, updated by by GP1(08h) - Display Mode
	uint32_t					scanlinePerVBlank;			//The number of scanlines per VBlank, updated by by GP1(08h) - Display Mode
	bool						newScanline;				//Set if a new Scanline has startes
	bool						newFrame;					//Set if a new Frame has started
	float						schedulerClockTicks;
	float						schedulerClockRatio;
		
	//GPU Memory Operation Status & Configurations
	gpu::VideoMemoryAccessState	vramAccessState;			//Current VRAM Access Status & Configuration
			
	//GPU Instruction Dictionaries and Functions
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
		uint8_t parameters;
	};

	std::vector<INSTRGP0> gp0InstrSet;						//Full GP0 Instruction Set
	std::vector<INSTRGP1> gp1InstrSet;						//Full GP1 Instruction Set

	//GP0 Instructions
	bool gp0_NoOperation();									//GP0(00h), No Operation, does nothing. Can be used for delay or synchronization.
	bool gp0_ClearTextureCache();							//GP0(01h), Clear Texture Cache
	bool gp0_FillVRam();									//GP0(02h), Fill VRAM
	bool gp0_InterruptRequest();							//GP0(1Fh), Interrupt Request
	bool gp0_Polygons();									//GP0(20h - 3Fh), Draw Polygons
	bool gp0_Lines();										//GP0(40h - 5Fh), Draw Lines
	bool gp0_Rectangles();									//GP0(60h - 7Fh), Draw Rectangles
	bool gp0_CopyVRam2VRam();								//GP0(80h - 9Fh), Copy VRAM to VRAM
	bool gp0_CopyRam2VRam();								//GP0(A0h - BFh), Copy RAM to VRAM
	bool gp0_CopyVRam2Ram();								//GP0(C0h - DFh), Copy VRAM to RAM
	bool gp0_DrawMode();									//GP0(E1h), Set Draw Mode
	bool gp0_TextureSetting();								//GP0(E2h), Set Texture Settings
	bool gp0_SetDrawAreaTop();								//GP0(E3h), Set Draw Area Top
	bool gp0_SetDrawAreaBottom();							//GP0(E4h), Set Draw Area Bottom
	bool gp0_SetDrawOffset();								//GP0(E5h), Set Draw Offset
	bool gp0_SetMaskBit();									//GP0(E6h), Set Mask Bit
	
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
	bool gp1_NewTextureDisable();
	bool gp1_Unknown();
	bool gp1_GetGpuInfo();

	void gp1_ResetStatus();
};

