#include "gpu.h"
#include "psx.h"
#include "timers.h"

GPU::GPU()
{
	//Reset Internal Registers
	gp0DataLatch = 0x00000000;
	gp1DataLatch = 0x00000000;
	gpuReadLatch = 0x00000000;
	gpuStat = 0x14802000; //TO BE CONFIRMED

	//Reset Internal Flags
	hBlank = false;
	vBlank = false;

	hCount = 0x00;
	vCount = 0x00;

	videoMode = VideoMode::NTSC;
	horizontalResolution1 = 0x0000;
	horizontalResolution2 = 0x0000;
	dotClockRatio = 10;				//Assuming 256 pixel per line is standard configuration
	verticalResolution = 0x0000;
	verticalInterlace = false;
	newScanline = false;
	newFrameReady = false;

	//VRAM & Video Settings
	memset(vRam, 0, sizeof(uint16_t) * VRAM_SIZE);
	memset(&displayArea, 0, sizeof(Quad<uint16_t>));
	memset(&displayOffset, 0, sizeof(Pair<uint16_t>));
	memset(&drawingArea, 0, sizeof(Quad<uint16_t>));
	memset(&drawingOffset, 0, sizeof(Pair<uint16_t>));
	memset(&textureMask, 0, sizeof(Pair<uint8_t>));
	memset(&textureOffset, 0, sizeof(Pair<uint8_t>));
	dmaDirection = 0;

	//Reset Internal Clock Counter
	gpuClockTicks = 0;

	//GPU Internal Status & Configurations
	recvCommand = false;
	recvParameters = false;
	gp0CommandAvailable = false;
	gp0Command = 0x00000000;
	gp0CommandParameters = 0;
	gp0CommandFifo = false;
	gp1CommandAvailable = false;
	gp1Command = 0x00000000;
	
	//ReceiveCommand Status
	recvCommand = false;
	recvParameters = false;

	//Memory Transfer Status & Configuration
	dataReadActive = false;
	dataWriteActive = false;
	memset(&dataDestination, 0, sizeof(Pair<uint16_t>));
	memset(&dataSource, 0, sizeof(Pair<uint16_t>));
	memset(&dataSize, 0, sizeof(Pair<uint16_t>));
	memset(&dataPointer, 0, sizeof(Pair<uint16_t>));

	//Init GPU Instruction Dictionaries

	gp0InstrSet =
	{
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"Clear Texture Cache", &GPU::gp0_ClearTextureCache, 0, false},
		{"Fill Rectangle in VRAM", &GPU::gp0_ClearScreen, 2, true},
		{"Unknown (NOP?)", &GPU::gp0_NoOperation, 0, true},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"Interrupt Request (IRQ1)", &GPU::gp0_InterruptRequest, 0, false},
		{"Monocrome 3 Point Polygon, Opaque", &GPU::gp0_Polygons, 3, true},
		{"Monocrome 3 Point Polygon, Opaque", &GPU::gp0_Polygons, 3, true},
		{"Monocrome 3 Point Polygon, Semi-Transparent", &GPU::gp0_Polygons, 3, true},
		{"Monocrome 3 Point Polygon, Semi-Transparent", &GPU::gp0_Polygons, 3, true},
		{"Textured 3 Point polygon, opaque, texture blending", &GPU::gp0_Polygons, 6, true},
		{"Textured 3 Point polygon, opaque, raw texture", &GPU::gp0_Polygons, 6, true},
		{"Textured 3 Point polygon, semi-transparent, texture blending", &GPU::gp0_Polygons, 6, true},
		{"Textured 3 Point polygon, semi-transparent, raw texture", &GPU::gp0_Polygons, 6, true},
		{"Monocrome 4 Point Polygon, Opaque", &GPU::gp0_Polygons, 4, true},
		{"Monocrome 4 Point Polygon, Opaque", &GPU::gp0_Polygons, 4, true},
		{"Monocrome 4 Point Polygon, Semi-Transparent", &GPU::gp0_Polygons, 4, true},
		{"Monocrome 4 Point Polygon, Semi-Transparent", &GPU::gp0_Polygons, 4, true},
		{"Textured 3 Point polygon, opaque, texture blending", &GPU::gp0_Polygons, 8, true},
		{"Textured 3 Point polygon, opaque, raw texture", &GPU::gp0_Polygons, 8, true},
		{"Textured 3 Point polygon, semi-transparent, texture blending", &GPU::gp0_Polygons, 8, true},
		{"Textured 3 Point polygon, semi-transparent, raw texture", &GPU::gp0_Polygons, 8, true},
		{"Shaded 3 Point polygon, opaque", &GPU::gp0_Polygons, 5, true},
		{"Shaded 3 Point polygon, opaque", &GPU::gp0_Polygons, 5, true},
		{"Shaded 3 Point polygon, semi-transparent", &GPU::gp0_Polygons, 5, true},
		{"Shaded 3 Point polygon, semi-transparent", &GPU::gp0_Polygons, 5, true},
		{"Shaded Textured 3 Point polygon, opaque, texture blending", &GPU::gp0_Polygons, 8, true},
		{"Shaded Textured 3 Point polygon, opaque, texture blending", &GPU::gp0_Polygons, 8, true},
		{"Shaded Textured 3 Point polygon, semi-transparent, texture blending", &GPU::gp0_Polygons, 8, true},
		{"Shaded Textured 3 Point polygon, semi-transparent, texture blending", &GPU::gp0_Polygons, 8, true},
		{"Shaded 4 Point polygon, opaque", &GPU::gp0_Polygons, 7, true},
		{"Shaded 4 Point polygon, opaque", &GPU::gp0_Polygons, 7, true},
		{"Shaded 4 Point polygon, semi-transparent", &GPU::gp0_Polygons, 7, true},
		{"Shaded 4 Point polygon, semi-transparent", &GPU::gp0_Polygons, 7, true},
		{"Shaded Textured 4 Point polygon, opaque, texture blending", &GPU::gp0_Polygons, 11, true},
		{"Shaded Textured 4 Point polygon, opaque, texture blending", &GPU::gp0_Polygons, 11, true},
		{"Shaded Textured 4 Point polygon, semi-transparent, texture blending", &GPU::gp0_Polygons, 11, true},
		{"Shaded Textured 4 Point polygon, semi-transparent, texture blending", &GPU::gp0_Polygons, 11, true},
		{"Monocrome line, opaque", &GPU::gp0_Lines, 2, true},
		{"", &GPU::gp0_NoOperation, 0, false},
		{"Monocrome line, semi-transparent", &GPU::gp0_Lines, 2, true},
		{"", &GPU::gp0_NoOperation, 0, false},
		{"", &GPU::gp0_NoOperation, 0, false},
		{"", &GPU::gp0_NoOperation, 0, false},
		{"", &GPU::gp0_NoOperation, 0, false},
		{"", &GPU::gp0_NoOperation, 0, false},
		{"Monocrome Polyline, opaque", &GPU::gp0_Lines, 255, true},
		{"", &GPU::gp0_NoOperation, 0, false},
		{"Monocrome Polyline, semi-transparent", &GPU::gp0_Lines, 255, true},
		{"", &GPU::gp0_NoOperation, 0, false},
		{"", &GPU::gp0_NoOperation, 0, false},
		{"", &GPU::gp0_NoOperation, 0, false},
		{"", &GPU::gp0_NoOperation, 0, false},
		{"", &GPU::gp0_NoOperation, 0, false},
		{"Shaded line, opaque", &GPU::gp0_Lines, 3, true},
		{"", &GPU::gp0_NoOperation, 0, false},
		{"Shaded line, semi-transparent", &GPU::gp0_Lines, 3, true},
		{"", &GPU::gp0_NoOperation, 0, false},
		{"", &GPU::gp0_NoOperation, 0, false},
		{"", &GPU::gp0_NoOperation, 0, false},
		{"", &GPU::gp0_NoOperation, 0, false},
		{"", &GPU::gp0_NoOperation, 0, false},
		{"Shaded Polyline, opaque", &GPU::gp0_Lines, 255, true},
		{"", &GPU::gp0_NoOperation, 0, false},
		{"Shaded Polyline. semi-transparent", &GPU::gp0_Lines, 255, true},
		{"", &GPU::gp0_NoOperation, 0, false},
		{"", &GPU::gp0_NoOperation, 0, false},
		{"", &GPU::gp0_NoOperation, 0, false},
		{"", &GPU::gp0_NoOperation, 0, false},
		{"", &GPU::gp0_NoOperation, 0, false},
		{"Monocrome Rectangle, variable size, opaque", &GPU::gp0_Rectangles, 2, true},
		{"", &GPU::gp0_NoOperation, 0, false},
		{"Monocrome Rectangle, variable size, semi-transparent", &GPU::gp0_Rectangles, 2, true},
		{"", &GPU::gp0_NoOperation, 0, false},
		{"Textured Rectangle, variable size, opaque, texture blending", &GPU::gp0_Rectangles, 3, true},
		{"Textured Rectangle, variable size, opaque, raw texture", &GPU::gp0_Rectangles, 3, true},
		{"Textured Rectangle, variable size, semi-transparent, texture blending", &GPU::gp0_Rectangles, 3, true},
		{"Textured Rectangle, variable size, semi-transparent, raw texture", &GPU::gp0_Rectangles, 3, true},
		{"Monocrome Rectangle, 1x1, opaque", &GPU::gp0_Rectangles, 1, true},
		{"", &GPU::gp0_NoOperation, 0, false},
		{"Monocrome Rectangle, 1x1, semi-transparent", &GPU::gp0_Rectangles, 1, true},
		{"", &GPU::gp0_NoOperation, 0, false},
		{"Textured Rectangle, 1x1, opaque, texture blending", &GPU::gp0_Rectangles, 2, true},
		{"Textured Rectangle, 1x1, opaque, raw texture", &GPU::gp0_Rectangles, 2, true},
		{"Textured Rectangle, 1x1, semi-transparent, texture blending", &GPU::gp0_Rectangles, 2, true},
		{"Textured Rectangle, 1x1, semi-transparent, raw texture", &GPU::gp0_Rectangles, 2, true},
		{"Monocrome Rectangle, 8x8, opaque", &GPU::gp0_Rectangles, 1, true},
		{"", &GPU::gp0_NoOperation, 0, false},
		{"Monocrome Rectangle, 8x8, semi-transparent", &GPU::gp0_Rectangles, 1, true},
		{"", &GPU::gp0_NoOperation, 0, false},
		{"Textured Rectangle, 8x8, opaque, texture blending", &GPU::gp0_Rectangles, 2, true},
		{"Textured Rectangle, 8x8, opaque, raw texture", &GPU::gp0_Rectangles, 2, true},
		{"Textured Rectangle, 8x8, semi-transparent, texture blending", &GPU::gp0_Rectangles, 2, true},
		{"Textured Rectangle, 8x8, semi-transparent, raw texture", &GPU::gp0_Rectangles, 2, true},
		{"Monocrome Rectangle, 16x16, opaque", &GPU::gp0_Rectangles, 1, true},
		{"", &GPU::gp0_NoOperation, 0, false},
		{"Monocrome Rectangle, 16x16, semi-transparent", &GPU::gp0_Rectangles, 1, true},
		{"", &GPU::gp0_NoOperation, 0, false},
		{"Textured Rectangle, 16x16, opaque, texture blending", &GPU::gp0_Rectangles, 2, true},
		{"Textured Rectangle, 16x16, opaque, raw texture", &GPU::gp0_Rectangles, 2, true},
		{"Textured Rectangle, 16x16, semi-transparent, texture blending", &GPU::gp0_Rectangles, 2, true},
		{"Textured Rectangle, 16x16, semi-transparent, raw texture", &GPU::gp0_Rectangles, 2, true},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_Vram2Vram, 3, true},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_Vram2Vram, 3, true},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_Vram2Vram, 3, true},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_Vram2Vram, 3, true},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_Vram2Vram, 3, true},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_Vram2Vram, 3, true},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_Vram2Vram, 3, true},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_Vram2Vram, 3, true},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_Vram2Vram, 3, true},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_Vram2Vram, 3, true},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_Vram2Vram, 3, true},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_Vram2Vram, 3, true},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_Vram2Vram, 3, true},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_Vram2Vram, 3, true},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_Vram2Vram, 3, true},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_Vram2Vram, 3, true},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_Vram2Vram, 3, true},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_Vram2Vram, 3, true},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_Vram2Vram, 3, true},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_Vram2Vram, 3, true},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_Vram2Vram, 3, true},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_Vram2Vram, 3, true},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_Vram2Vram, 3, true},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_Vram2Vram, 3, true},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_Vram2Vram, 3, true},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_Vram2Vram, 3, true},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_Vram2Vram, 3, true},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_Vram2Vram, 3, true},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_Vram2Vram, 3, true},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_Vram2Vram, 3, true},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_Vram2Vram, 3, true},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_Vram2Vram, 3, true},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_Ram2Vram, 2, true},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_Ram2Vram, 2, true},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_Ram2Vram, 2, true},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_Ram2Vram, 2, true},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_Ram2Vram, 2, true},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_Ram2Vram, 2, true},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_Ram2Vram, 2, true},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_Ram2Vram, 2, true},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_Ram2Vram, 2, true},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_Ram2Vram, 2, true},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_Ram2Vram, 2, true},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_Ram2Vram, 2, true},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_Ram2Vram, 2, true},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_Ram2Vram, 2, true},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_Ram2Vram, 2, true},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_Ram2Vram, 2, true},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_Ram2Vram, 2, true},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_Ram2Vram, 2, true},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_Ram2Vram, 2, true},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_Ram2Vram, 2, true},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_Ram2Vram, 2, true},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_Ram2Vram, 2, true},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_Ram2Vram, 2, true},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_Ram2Vram, 2, true},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_Ram2Vram, 2, true},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_Ram2Vram, 2, true},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_Ram2Vram, 2, true},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_Ram2Vram, 2, true},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_Ram2Vram, 2, true},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_Ram2Vram, 2, true},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_Ram2Vram, 2, true},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_Ram2Vram, 2, true},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_Vram2Ram, 2, true},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_Vram2Ram, 2, true},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_Vram2Ram, 2, true},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_Vram2Ram, 2, true},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_Vram2Ram, 2, true},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_Vram2Ram, 2, true},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_Vram2Ram, 2, true},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_Vram2Ram, 2, true},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_Vram2Ram, 2, true},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_Vram2Ram, 2, true},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_Vram2Ram, 2, true},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_Vram2Ram, 2, true},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_Vram2Ram, 2, true},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_Vram2Ram, 2, true},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_Vram2Ram, 2, true},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_Vram2Ram, 2, true},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_Vram2Ram, 2, true},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_Vram2Ram, 2, true},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_Vram2Ram, 2, true},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_Vram2Ram, 2, true},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_Vram2Ram, 2, true},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_Vram2Ram, 2, true},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_Vram2Ram, 2, true},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_Vram2Ram, 2, true},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_Vram2Ram, 2, true},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_Vram2Ram, 2, true},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_Vram2Ram, 2, true},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_Vram2Ram, 2, true},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_Vram2Ram, 2, true},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_Vram2Ram, 2, true},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_Vram2Ram, 2, true},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_Vram2Ram, 2, true},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"Draw Mode Setting", &GPU::gp0_DrawMode, 0, false},
		{"Texture Window Setting", &GPU::gp0_TextureSetting, 0, false},
		{"Set Drawing Area Top Left", &GPU::gp0_SetDrawAreaTop, 0, false},
		{"Set Drawing Area Bottom Right", &GPU::gp0_SetDrawAreaBottom, 0, false},
		{"Set Drawing Offset", &GPU::gp0_SetDrawOffset, 0, false},
		{"Mask Bit Setting", &GPU::gp0_SetMaskBit, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false}
	};

	gp1InstrSet =
	{
		{"Reset GPU", &GPU::gp1_ResetGpu},
		{"Reset Command Buffer", &GPU::gp1_ResetFifo},
		{"Ack GPU Interrupt", &GPU::gp1_AckInterrupt},
		{"Display Enable", &GPU::gp1_DisplayEnable},
		{"DMA Direction", &GPU::gp1_DmaDirection},
		{"Start Display Area VRAM", &GPU::gp1_StartDisplayArea},
		{"Horizontal Display Range", &GPU::gp1_HDisplayRange},
		{"Vertical Display Range", &GPU::gp1_VDisplayRange},
		{"Display Mode", &GPU::gp1_DisplayMode},
		{"New Texture Disable", &GPU::gp1_TextureDisable},
		{"Unknown", &GPU::gp1_Unknown},
		{"Unknown", &GPU::gp1_Unknown},
		{"Unknown", &GPU::gp1_Unknown},
		{"Unknown", &GPU::gp1_Unknown},
		{"Unknown", &GPU::gp1_Unknown},
		{"Unknown", &GPU::gp1_Unknown},
		{"Get GPU Info", &GPU::gp1_GetGpuInfo},
		{"Get GPU Info", &GPU::gp1_GetGpuInfo},
		{"Get GPU Info", &GPU::gp1_GetGpuInfo},
		{"Get GPU Info", &GPU::gp1_GetGpuInfo},
		{"Get GPU Info", &GPU::gp1_GetGpuInfo},
		{"Get GPU Info", &GPU::gp1_GetGpuInfo},
		{"Get GPU Info", &GPU::gp1_GetGpuInfo},
		{"Get GPU Info", &GPU::gp1_GetGpuInfo},
		{"Get GPU Info", &GPU::gp1_GetGpuInfo},
		{"Get GPU Info", &GPU::gp1_GetGpuInfo},
		{"Get GPU Info", &GPU::gp1_GetGpuInfo},
		{"Get GPU Info", &GPU::gp1_GetGpuInfo},
		{"Get GPU Info", &GPU::gp1_GetGpuInfo},
		{"Get GPU Info", &GPU::gp1_GetGpuInfo},
		{"Get GPU Info", &GPU::gp1_GetGpuInfo},
		{"Get GPU Info", &GPU::gp1_GetGpuInfo},
		{"Special Texture Disable", &GPU::gp1_TextureDisable},
		{"Unknown", &GPU::gp1_Unknown},
		{"Unknown", &GPU::gp1_Unknown},
		{"Unknown", &GPU::gp1_Unknown},
		{"Unknown", &GPU::gp1_Unknown},
		{"Unknown", &GPU::gp1_Unknown},
		{"Unknown", &GPU::gp1_Unknown},
		{"Unknown", &GPU::gp1_Unknown},
		{"Unknown", &GPU::gp1_Unknown},
		{"Unknown", &GPU::gp1_Unknown},
		{"Unknown", &GPU::gp1_Unknown},
		{"Unknown", &GPU::gp1_Unknown},
		{"Unknown", &GPU::gp1_Unknown},
		{"Unknown", &GPU::gp1_Unknown},
		{"Unknown", &GPU::gp1_Unknown},
		{"Unknown", &GPU::gp1_Unknown},
		{"Unknown", &GPU::gp1_Unknown},
		{"Unknown", &GPU::gp1_Unknown},
		{"Unknown", &GPU::gp1_Unknown},
		{"Unknown", &GPU::gp1_Unknown},
		{"Unknown", &GPU::gp1_Unknown},
		{"Unknown", &GPU::gp1_Unknown},
		{"Unknown", &GPU::gp1_Unknown},
		{"Unknown", &GPU::gp1_Unknown},
		{"Unknown", &GPU::gp1_Unknown},
		{"Unknown", &GPU::gp1_Unknown},
		{"Unknown", &GPU::gp1_Unknown},
		{"Unknown", &GPU::gp1_Unknown},
		{"Unknown", &GPU::gp1_Unknown},
		{"Unknown", &GPU::gp1_Unknown},
		{"Unknown", &GPU::gp1_Unknown},
		{"Unknown", &GPU::gp1_Unknown}
	};
}

GPU::~GPU()
{
}

bool GPU::reset()
{
	//Reset Internal Registers
	gp0DataLatch = 0x00000000;
	gp1DataLatch = 0x00000000;
	gpuReadLatch = 0x00000000;
	gpuStat = 0x14802000; //TO BE CONFIRMED

	//Reset Internal Flags
	hBlank = false;
	vBlank = false;

	hCount = 0x00;
	vCount = 0x00;

	videoMode = VideoMode::NTSC;
	horizontalResolution1 = 0x0000;
	horizontalResolution2 = 0x0000;
	verticalResolution = 0x0000;
	verticalInterlace = false;
	newScanline = false;
	newFrameReady = false;

	//VRAM & Video Settings
	memset(vRam, 0, sizeof(uint16_t) * VRAM_SIZE);
	memset(&displayArea, 0, sizeof(Quad<uint16_t>));
	memset(&displayOffset, 0, sizeof(Pair<uint16_t>));
	memset(&drawingArea, 0, sizeof(Quad<uint16_t>));
	memset(&drawingOffset, 0, sizeof(Pair<uint16_t>));
	memset(&textureMask, 0, sizeof(Pair<uint8_t>));
	memset(&textureOffset, 0, sizeof(Pair<uint8_t>));
	dmaDirection = 0;

	//Reset Internal Clock Counter
	gpuClockTicks = 0;

	//GPU Internal Status & Configurations
	recvCommand = false;
	recvParameters = false;
	gp0CommandAvailable = false;
	gp0Command = 0x00000000;
	gp0CommandParameters = 0;
	gp0CommandFifo = false;
	gp1CommandAvailable = false;
	gp1Command = 0x00000000;

	//ReceiveCommand Status
	recvCommand = false;
	recvParameters = false;

	//Memory Transfer Status & Configuration
	dataReadActive = false;
	dataWriteActive = false;
	memset(&dataDestination, 0, sizeof(Pair<uint16_t>));
	memset(&dataSource, 0, sizeof(Pair<uint16_t>));
	memset(&dataSize, 0, sizeof(Pair<uint16_t>));
	memset(&dataPointer, 0, sizeof(Pair<uint16_t>));

	return true;
}

//-----------------------------------------------------------------------------------------------------
// 
//                               HELPER FUNCTIONS
// 
//-----------------------------------------------------------------------------------------------------
bool GPU::isFrameReady()
{
	bool status;

	status = newFrameReady;
	newFrameReady = false;

	return status;
}

void GPU::writeVRAM(uint32_t& data)
{
	uint16_t h, l;
	h = static_cast<uint16_t>((data & 0xffff0000) >> 16);
	l = static_cast<uint16_t>((data & 0x0000ffff));

	vRam[dataPointer.y][dataPointer.x] = l;
	vRam[dataPointer.y][dataPointer.x + 1] = h;

	//Update Data Pointer in VRAM
	dataPointer.x += 2;
	if (dataPointer.x >= dataDestination.x + dataSize.x)
	{
		dataPointer.x = dataDestination.x;
		dataPointer.y++;
	}

	if (dataPointer.y >= dataDestination.y + dataSize.y)
		dataWriteActive = false;

	return;
}

uint32_t GPU::readVRAM()
{
	uint32_t data;
	uint16_t h, l;

	l = vRam[dataPointer.y][dataPointer.x];
	h = vRam[dataPointer.y][dataPointer.x + 1];
	data = (h << 16) | l;

	//Update Data Pointer in VRAM
	dataPointer.x += 2;
	if (dataPointer.x >= dataDestination.x + dataSize.x)
	{
		dataPointer.x = dataDestination.x;
		dataPointer.y++;
	}

	if (dataPointer.y >= dataDestination.y + dataSize.y)
		dataReadActive = false;

	return data;
}

//-----------------------------------------------------------------------------------------------------
// 
//                               GPU Interface
// 
//-----------------------------------------------------------------------------------------------------
bool GPU::clock()
{
	auto updateVHBlank = [&](const unsigned int active_ticks, const unsigned int hblank_ticks, const unsigned int visible_scanlines, const unsigned int vblank_scanlines)
	{
		hBlank = !(hCount < active_ticks);
		vBlank = !(vCount < visible_scanlines);

		hCount++;
		if (hCount >= (active_ticks + hblank_ticks))
		{
			hCount = 0;
			vCount++;
			newScanline = true;
			hBlank = false;		//Reset hBlank immediately on a new line, is it really needed?
			
			//Generate hBlank Clock, not sure if it has to be triggered as soon as exit the visible area 
			psx->timers.clock(ClockSource::hBlank);
		}

		if (vCount >= (visible_scanlines + vblank_scanlines))
		{
			vCount = 0;
			newFrameReady = true;
			vBlank = false;		//Reset vBlank immediately in order to update GPUSTAT.31 on a new frame correctly otherwise is always reset to zero.
			//printf("GPU -------------NEW FRAME!\n");
		}

		return 0;
	};
	auto gp0RunCommand = [&]()
	{
		uint32_t opcode;

		if (gp0CommandFifo)
		{
			//Available GP0 Command in the FIFO	
			fifo.pop(gp0Command);
			opcode = gp0Command >> 24;
			
			(this->*gp0InstrSet[opcode].operate)();
			//printf("Command GP0(%02xh): %s\n", opcode, gp0InstrSet[opcode].mnemonic.c_str());
		}
		else
		{
			//Available GP0 Command not in the FIFO
			opcode = gp0Command >> 24;
			(this->*gp0InstrSet[opcode].operate)();
			//printf("Command GP0(%02xh): %s\n", opcode, gp0InstrSet[opcode].mnemonic.c_str());
		}
	
		return 0;
	};
	auto gp1RunCommand = [&]()
	{
		uint8_t opcode;
		opcode = (gp1Command >> 24) & 0x3f;			//Extract Command Opcode, masked since from 0x40 are all mirrored
		(this->*gp1InstrSet[opcode].operate)();		//GP1 Command are always executed immediately
		//printf("Command GP1(%02xh): %s\n", opcode, gp1InstrSet[opcode].mnemonic.c_str());
		return 0;
	};

	newScanline = false;
	newFrameReady = false;

	//Update Vertical and Horizontal Blank Flag according to Video Mode
	switch (videoMode)
	{
		case VideoMode::NTSC:
			updateVHBlank(NTSC_ACTIVE_GPU_TICK, NTSC_HBLANK_GPU_TICK, NTSC_VISIBLE_SCANLINES, NTSC_VBLANK_SCANLINES);
			break;

		case VideoMode::PAL:
			updateVHBlank(PAL_ACTIVE_GPU_TICK, PAL_HBLANK_GPU_TICK, PAL_VISIBLE_SCANLINES, PAL_VBLANK_SCANLINES);
			break;
	}

	//Update GPUSTAT.31
	//  - Toggle at every new scanline if GPUSTAT.19 = 0
	//  - Toggle at every new frame if GPUSTAT.19 = 1
	//  - Always Zero during vBlank
	if (newScanline & !static_cast<bool>((gpuStat & (1UL << 19)))) { gpuStat ^= (1UL << 31); };
	if (newFrameReady & static_cast<bool>((gpuStat & (1UL << 19)))) { gpuStat ^= (1UL << 31); };
	if (vBlank) { gpuStat &= ~(1UL << 31); };

	//Update GPUSTAT.28
	//  - Set to 0 after receiving both GP0/1 Command and all GP0/1 Parameters (GPU is Busy)
	//  - Set to 1 when GPU is ready to receive a new Command and Parameters (previous command completed execution)
	//  For Polygons and Lines GPUSTAT.28 get reset immediately after receiving the commands. Not implemented yet
	gpuStat = (gpuStat & ~(1UL << 28)) | (static_cast<uint32_t>(!(recvCommand & recvParameters)) << 28);

	//Update GPUSTAT.27
	//  - Set to 1 by GP0(C0h) - DMA transfer from VRAM to RAM
	//  - Set to 0 when all data has been transferred to RAM
	gpuStat = (gpuStat & ~(1UL << 27)) | (static_cast<uint32_t>(dataReadActive) << 27);
	
	//Update GPUSTAT.26
	//  - Set to 0 after receiving a GP0 Command or when GPU is Busy
	//  - Set to 1 when GPU is ready to receive a new command (previous command completed execution)
	//if (recvCommand) { gpuStat &= ~(1UL << 26); } //Reset to 0
	gpuStat = (gpuStat & ~(1UL << 26)) | (static_cast<uint32_t>(!recvCommand) << 26);

	//Update GPUSTAT.25
	// - When GPUSTAT.29-30 = 0 --->Always zero(0)
	// - When GPUSTAT.29-30 = 1 --->FIFO State(0 = Full, 1 = Not Full)
	// - When GPUSTAT.29-30 = 2 --->Same as GPUSTAT.28
	// - When GPUSTAT.29-30 = 3 --->Same as GPUSTAT.27
	// GPUSTAT.29-30 is set by GP1(04h) DmaDirection (0=Off, 1=FIFO, 2=RAM to VRAM, 3=VRAM to RAM)
	uint32_t data;
	switch (dmaDirection)
	{
	case 0:
		gpuStat &= ~(1UL << 25); //Reset to 0
		break;

	case 1:
		data = static_cast<uint32_t>(!fifo.isfull());
		gpuStat = (gpuStat & ~(1UL << 25)) | (data << 25);
		break;

	case 2:
		data = gpuStat & (1UL << 28) >> 28;
		gpuStat = (gpuStat & ~(1UL << 25)) | (data << 25);
		break;

	case 3:
		data = gpuStat & (1UL << 27) >> 27;
		gpuStat = (gpuStat & ~(1UL << 25)) | (data << 25);
		break;
	}

	//Run Available Command, either GP0 or GP1 Command
	if (gp0CommandAvailable) { gp0RunCommand(); };
	if (gp1CommandAvailable) { gp1RunCommand(); };

	//Generate Dot Clock Signal
	if (!(gpuClockTicks % dotClockRatio))
	{
		psx->timers.clock(ClockSource::Dot);
	}
	gpuClockTicks++;

	return true;
}

bool GPU::setParameter(uint32_t addr, uint32_t& data, uint8_t bytes)
{
	auto gp0ReceiveCommand = [&](uint32_t word)
	{
		if (!recvCommand)
		{
			//printf("GPU - GP0 Command   : 0x%08x  (Clk: %lld) [%d, %d]\n", word, clockCounter, dmaDirection, gpuDataTransferActive);

			uint32_t opcode = word >> 24;
			recvCommand = true;						
			gp0CommandParameters = gp0InstrSet[opcode].parameters;
			gp0CommandFifo = gp0InstrSet[opcode].fifo;

			if (gp0CommandFifo)
				fifo.push(word);
			else
				gp0Command = word;

			if (gp0CommandParameters == 0)
			{
				recvParameters = true;
				gp0CommandAvailable = true;
			}

			return 0;
		}

		if (!recvParameters)
		{
			//printf("GPU - GP0 Parameter : 0x%08x  (Clk: %lld) [%d, %d]\n", data, clockCounter, dmaDirection, gpuDataTransferActive);
			
			fifo.push(word);
			gp0CommandParameters--;
			if (gp0CommandParameters == 0)
			{
				recvParameters = true;
				gp0CommandAvailable = true;
			}
		}
				
		return 0;
	};
	auto gp1ReceiveCommand = [&](uint32_t word)
	{
		//printf("GPU - GP1 Command   : 0x%08x  (Clk: %lld)\n", word, clockCounter);
		//All GP1 Commands don't have parameters and don't use FIFO
		recvCommand = true;
		recvParameters = true;
		gp1CommandAvailable = true;
		gp1Command = word;
		
		return 0;
	};
	auto gp0ReceiveRamData = [&](uint32_t word)
	{
		//printf("GPU - RAM to VRAM   : 0x%08x  (Clk: %lld) [%d, %d]\n", word, clockCounter, dmaDirection, gpuDataTransferActive);
		writeVRAM(word);
	
		return 0;
	};
	
	switch (addr)
	{
	case 0x1f801810:	//--------------------------GP0 Command
		gp0DataLatch = data;						//Rendering and VRAM Access
		if (dataWriteActive && (dmaDirection == 2 || dmaDirection == 0))
			//dataWriteActive is set by GP0(A0h)
			//Data Transfer could be either thru:
			//	- CPU, GPUSTAT.29-30 (DMA Direction) = 0  "DMA Off"
			//	- DMA, GPUSTAT.29-30 (DMA Direction) = 2  "DMA RAM to VRAM"
			gp0ReceiveRamData(data);
		else
			gp0ReceiveCommand(data);

		break;
	case 0x1f801814:	//--------------------------GP1 Command
		gp1DataLatch = data;						//Display Control
		gp1ReceiveCommand(data);					//Receive GP1 Command and update GPUSTAT flags
		break;

	default:
		printf("GPU - Unknown Parameter Set addr: 0x%08x (%d), data: 0x%08x\n", addr, bytes, data);
		return false;
	}
	return true;
}

uint32_t GPU::getParameter(uint32_t addr, uint8_t bytes)
{
	uint32_t data;
	
	switch (addr)
	{
	case 0x1f801810:
		data = gpuReadLatch;	//--------------------------------------Response from GP0 and GP1 commands
		if (dataReadActive && (dmaDirection == 3 || dmaDirection == 0))
			//gpuDataTransferActive is set by GP0(A0h) and GP0(C0h)
			//Data Transfer could be either thru:
			//	- CPU, GPUSTAT.29-30 (DMA Direction) = 0  "DMA Off"
			//	- DMA, GPUSTAT.29-30 (DMA Direction) = 3  "DMA RAM to VRAM"
			data = readVRAM();
		printf("GPU - GP0/1 Response: 0x%08x  (Clk: %ld) [%d, %d]\n", data, gpuClockTicks, dmaDirection, dataReadActive);
		break;
	case 0x1f801814:
		data = gpuStat;			//--------------------------------------GPU Status Register
		//printf("GPU - GPUSTAT Read  : 0x%08x  (Clk: %lld)\n", data, clockCounter);
		break;

	default:
		printf("GPU - Unknown Parameter Get addr: 0x%08x (%d)\n", addr, bytes);
		return 0x0;
	}

	return data;
}
//-----------------------------------------------------------------------------------------------------
// 
//                               GP0 Full Instruction Set
// 
//-----------------------------------------------------------------------------------------------------

bool GPU::gp0_Lines()
{
	//Reset GPUSTAT Flag to receive next GP0 command
	gp0_ResetStatus();

	return false;
}

bool GPU::gp0_Rectangles()
{
	//Reset GPUSTAT Flag to receive next GP0 command
	gp0_ResetStatus();

	return false;
}

bool GPU::gp0_Vram2Vram()
{
	//GP0(80h)
	//Copy Rectangle VRAM to VRAM
	uint32_t	param;

	fifo.pop(param);		//Source Coord     (YyyyXxxxh); Xpos counted in halfwords, Ypos counted in rows
	dataSource.x = static_cast<uint16_t>((param & 0x0000ffff));
	dataSource.y = static_cast<uint16_t>((param & 0xffff0000) >> 16);

	fifo.pop(param);		//Destination Coord(YyyyXxxxh); Xpos counted in halfwords, Ypos counted in rows
	dataDestination.x = static_cast<uint16_t>((param & 0x0000ffff));
	dataDestination.y = static_cast<uint16_t>((param & 0xffff0000) >> 16);

	fifo.pop(param);		//Width+Height     (YsizXsizh); Xsiz counted in halfwords, Ysiz counted in rows
	dataSize.x = static_cast<uint16_t>((param & 0x0000ffff));
	dataSize.y = static_cast<uint16_t>((param & 0xffff0000) >> 16);

	//TODO
	//Implement copy from area to area

	//Reset GPUSTAT Flag to receive next GP0 command
	gp0_ResetStatus();

	return false;
}

bool GPU::gp0_Ram2Vram()
{
	//GP0(A0h)
	//Copy Rectangle RAM to VRAM
	uint32_t	param;
	
	fifo.pop(param);		//Destination Coord(YyyyXxxxh); Xpos counted in halfwords, Ypos counted in rows
	dataDestination.x = static_cast<uint16_t>((param & 0x0000ffff));
	dataDestination.y = static_cast<uint16_t>((param & 0xffff0000) >> 16);

	fifo.pop(param);		//Width+Height     (YsizXsizh); Xsiz counted in halfwords, Ysiz counted in rows
	dataSize.x = static_cast<uint16_t>((param & 0x0000ffff));
	dataSize.y = static_cast<uint16_t>((param & 0xffff0000) >> 16);

	//Set Internal Transfer Status and Size
	dataPointer.x = dataDestination.x;
	dataPointer.y = dataDestination.y;
	dataWriteActive = true;
			 
	//Reset GPUSTAT Flag to receive next GP0 command
	gp0_ResetStatus();

	return false;
}

bool GPU::gp0_Vram2Ram()
{
	//GP0(C0h)
	//Copy Rectangle VRAM to RAM
	uint32_t	param;

	fifo.pop(param);		//Source Coord     (YyyyXxxxh); Xpos counted in halfwords, Ypos counted in rows
	dataSource.x = static_cast<uint16_t>((param & 0x0000ffff));
	dataSource.y = static_cast<uint16_t>((param & 0xffff0000) >> 16);

	fifo.pop(param);		//Width+Height     (YsizXsizh); Xsiz counted in halfwords, Ysiz counted in rows	
	dataSize.x = static_cast<uint16_t>((param & 0x0000ffff));
	dataSize.y = static_cast<uint16_t>((param & 0xffff0000) >> 16);

	//Set Internal Transfer Status and Size
	dataPointer.x = dataSource.x;
	dataPointer.y = dataSource.y;
	dataReadActive = true;
	 
	//Reset GPUSTAT Flag to receive next GP0 command
	gp0_ResetStatus();

	return false;
}

bool GPU::gp0_NoOperation()
{
	//Reset GPUSTAT Flag to receive next GP0 command
	gp0_ResetStatus();

	return true;
}

bool GPU::gp0_ClearTextureCache()
{
	//Reset GPUSTAT Flag to receive next GP0 command
	gp0_ResetStatus();

	return false;
}

bool GPU::gp0_ClearScreen()
{
	//Reset GPUSTAT Flag to receive next GP0 command
	gp0_ResetStatus();

	return false;
}

bool GPU::gp0_InterruptRequest()
{
	//Reset GPUSTAT Flag to receive next GP0 command
	gp0_ResetStatus();

	return false;
}

bool GPU::gp0_Polygons()
{
	//Reset GPUSTAT Flag to receive next GP0 command
	gp0_ResetStatus();

	return false;
}

bool GPU::gp0_DrawMode()
{
	//GP0(E1h)
	//Draw Mode Settings, Texpage. Modify various GPUSTAT bits

	uint32_t data;

	//GPUSTAT.0-3 << GP0DATA.0-3								Texture page X Base (N x 64) in halfwords
	data = gp0DataLatch & 0x0000000f;							//Extract bit value from GP0 Command
	gpuStat = (gpuStat & ~(0x0000000f)) | (data);				//Set GPUSTAT.0-3 to data value

	//GPUSTAT.4 << GP0DATA.4									Texture page Y Base (N x 256) in halfwords
	data = (gp0DataLatch & 0x00000010) >> 4;					//Extract bit value from GP0 Command
	gpuStat = (gpuStat & ~(0x00000001 << 4)) | (data << 4);		//Set GPUSTAT.4 to data value

	//GPUSTAT.5-6 << GP0DATA.5-6								Semi Transparency (0 = B/2+F/2, 1 = B+F, 2 = B-F, 3 = B+F/4)
	data = (gp0DataLatch & 0x000000060) >> 5;					//Extract bit value from GP0 Command
	gpuStat = (gpuStat & ~(0x00000003 << 5)) | (data << 5);		//Set GPUSTAT.5-6 to data value

	//GPUSTAT.7-8 << GP0DATA.7-8								Texture page colors (0 = 4bit, 1 = 8bit, 2 = 15bit, 3 = reserved)
	data = (gp0DataLatch & 0x00000180) >> 7;					//Extract bit value from GP0 Command
	gpuStat = (gpuStat & ~(0x00000003 << 7)) | (data << 7);		//Set GPUSTAT.7-8 to data value

	//GPUSTAT.9 << GP0DATA.9									Dither 24bit to 15bit (0 = off, 1 = enabled)
	data = (gp0DataLatch & 0x00000200) >> 9;					//Extract bit value from GP0 Command
	gpuStat = (gpuStat & ~(0x00000001 << 9)) | (data << 9);		//Set GPUSTAT.9 to data value

	//GPUSTAT.10 << GP0DATA.10									Drawing to display area (0 = prohibited, 1 = allowed)
	data = (gp0DataLatch & 0x00000400) >> 10;					//Extract bit value from GP0 Command
	gpuStat = (gpuStat & ~(0x00000001 << 10)) | (data << 10);	//Set GPUSTAT.10 to data value

	//GPUSTAT.15 << GP0DATA.11									Texture Disable (0 = normal, 1 = disable)
	data = (gp0DataLatch & 0x00000800) >> 11;					//Extract bit value from GP0 Command
	gpuStat = (gpuStat & ~(0x00000001 << 15)) | (data << 15);	//Set GPUSTAT.15 to data value

	//Reset GPUSTAT Flag to receive next GP0 command
	gp0_ResetStatus();

	return true;
}

bool GPU::gp0_TextureSetting()
{
	//GP0(E2h)
	//Texture Window Settings

	uint32_t data;

	//GP0DATA.0-4												Texture windows Mask X (in 8 pixel steps)
	data = (gp0DataLatch & 0x0000001f);							//Extract bit value from GP0 Command
	 textureMask.x = static_cast<uint8_t>(data);					

	//GP0DATA.5-9												Texture windows Mask Y (in 8 pixel steps)
	data = (gp0DataLatch & 0x000003e0) >> 5;					//Extract bit value from GP0 Command
	textureMask.y = static_cast<uint8_t>(data);

	//GP0DATA.10-14												Texture windows Offset X (in 8 pixel steps)
	data = (gp0DataLatch & 0x00007c00) >> 10;					//Extract bit value from GP0 Command
	textureOffset.x = static_cast<uint8_t>(data);

	//GP0DATA.15-19												Texture windows Offset- Y (in 8 pixel steps)
	data = (gp0DataLatch & 0x000f8000) >> 15;					//Extract bit value from GP0 Command
	textureOffset.y = static_cast<uint8_t>(data);

	//Reset GPUSTAT Flag to receive next GP0 command
	gp0_ResetStatus();

	return true;
}

bool GPU::gp0_SetDrawAreaTop()
{
	//GP0(E3h)
	//Set Drawing Area top left

	uint32_t data;

	//GP0DATA.0-9												X Coordinates top left
	data = (gp0DataLatch & 0x000003ff);							//Extract bit value from GP0 Command
	drawingArea.x1 = static_cast<uint16_t>(data);

	//GP0DATA.10-19												Y Coordinates top left
	data = (gp0DataLatch & 0x000ffc00) >> 10;					//Extract bit value from GP0 Command
	drawingArea.y1 = static_cast<uint16_t>(data);

	//Reset GPUSTAT Flag to receive next GP0 command
	gp0_ResetStatus();

	return true;
}

bool GPU::gp0_SetDrawAreaBottom()
{
	//GP0(E4h)
	//Set Drawing Area top left

	uint32_t data;

	//GP0DATA.0-9												X Coordinates bottom right
	data = (gp0DataLatch & 0x000003ff);							//Extract bit value from GP0 Command
	drawingArea.x2 = static_cast<uint16_t>(data);

	//GP0DATA.10-19												Y Coordinates bottom right
	data = (gp0DataLatch & 0x000ffc00) >> 10;					//Extract bit value from GP0 Command
	drawingArea.y2 = static_cast<uint16_t>(data);

	//Reset GPUSTAT Flag to receive next GP0 command
	gp0_ResetStatus();

	return true;
}

bool GPU::gp0_SetDrawOffset()
{
	//GP0(E5h)
	//Set Drawing Offset

	uint32_t data;

	//GP0DATA.0-10												X Offset
	data = (gp0DataLatch & 0x000007ff);							//Extract bit value from GP0 Command
	drawingOffset.x = static_cast<uint16_t>(data);

	//GP0DATA.11-21												Y Offset
	data = (gp0DataLatch & 0x003ff800) >> 11;					//Extract bit value from GP0 Command
	drawingOffset.y = static_cast<uint16_t>(data);

	//Reset GPUSTAT Flag to receive next GP0 command
	gp0_ResetStatus();

	return true;
}

bool GPU::gp0_SetMaskBit()
{
	//GP0(E6h)
	//Mask Bit Setting

	uint32_t data;

	//GPUSTAT.11 << GP0DATA.0									Set Mask while drawing (0 = disable, 1 = enable)
	data = gp0DataLatch & 0x00000001;							//Extract bit value from GP0 Command
	gpuStat = (gpuStat & ~(0x00000001 << 11)) | (data << 11);	//Set GPUSTAT.11 to data value

	//GPUSTAT.12 << GP0DATA.1									Texture page Y Base (N x 256) in halfwords
	data = (gp0DataLatch & 0x00000002) >> 1;					//Extract bit value from GP1 Command
	gpuStat = (gpuStat & ~(0x00000001 << 12)) | (data << 12);	//Set GPUSTAT.12 to data value
	
	//Reset GPUSTAT Flag to receive next GP0 command
	gp0_ResetStatus();

	return true;
}

void GPU::gp0_ResetStatus()
{
	recvCommand = false;
	recvParameters = false;
	gp0CommandAvailable = 0;

	fifo.flush();	//TEMPORARY!!!!!!!!!!!!!!!!!!!!!!!!
}

//-----------------------------------------------------------------------------------------------------
// 
//                               GP1 Full Instruction Set
// 
//-----------------------------------------------------------------------------------------------------
bool GPU::gp1_ResetGpu()
{
	//GP1(00h)
	//Reset GPUSTAT to initial value except for GPUSTAT.15 which remains untouched

	uint32_t data;

	data = gpuStat & (0x00000001 << 15);					//Extract GPUSTAT.15 value;
	gpuStat = (0x14802000 & ~(0x00000001 << 15)) | data;	//Reset GPUSTAT

	//Reset GPUSTAT Flag to receive next GP1 command
	gp1_ResetStatus();

	return true;
}

bool GPU::gp1_ResetFifo()
{
	//GP1(01h)
	//Flush Command FIFO Buffer 
	
	fifo.flush();

	//Reset GPUSTAT Flag to receive next GP1 command
	gp1_ResetStatus();

	return true;
}

bool GPU::gp1_AckInterrupt()
{
	//GP1(02h)
	//Acnowledge GPU Interrupt, this flag in set by GP0(1Fh)

	//GPUSTAT.24 << 0
	gpuStat &= ~(0x00000001 << 24);

	//Reset GPUSTAT Flag to receive next GP1 command
	gp1_ResetStatus();

	return true;
}

bool GPU::gp1_DisplayEnable()
{
	//GP1(03h)
	//Display Enable (0 = on, 1 = off)
		
	uint32_t data;

	//GPUSTAT.23 << GP1DATA.0	
	data = gp1DataLatch & 0x00000001;							//Extract bit value from GP1 Command
	gpuStat = (gpuStat & ~(0x00000001 << 23)) | (data << 23);	//Set GPUSTAT.23 to data value

	//Reset GPUSTAT Flag to receive next GP1 command
	gp1_ResetStatus();
	
	return true;
}

bool GPU::gp1_DmaDirection()
{
	//GP1(04h)
	//DMA Direction / Data Requestt (0 = off, 1 = FIFO, 2 =  RAM to VRAM, 3 = VRAM to RAM)
	
	uint32_t data;

	//GPUSTAT.29-30 << GP1DATA.0-1
	data = gp1DataLatch & 0x00000003;							//Extract bit value from GP1 Command
	gpuStat = (gpuStat & ~(0x00000003 << 29)) | (data << 29);	//Set GPUSTAT.29-30 to data value

	dmaDirection = data;

	//Reset GPUSTAT Flag to receive next GP1 command
	gp1_ResetStatus();

	return true;
}

bool GPU::gp1_StartDisplayArea()
{
	//GP1(05h)
	//Start of Display Area in VRAM (gp1Data.0-9 = X, gp1Data.10-18 = Y)
	 
	displayOffset.x = gp1DataLatch & 0x000003ff;
	displayOffset.y = (gp1DataLatch & 0x0007fc00) >> 10;

	//Reset GPUSTAT Flag to receive next GP1 command
	gp1_ResetStatus();

	return true;
}

bool GPU::gp1_HDisplayRange()
{
	//GP1(06h)
	//Horizontal Display Range on Screen (gp1Data.0-11 = X1, gp1Data.12-23 = X2)
		
	displayArea.x1 = gp1DataLatch & 0x00000fff;
	displayArea.x2 = (gp1DataLatch & 0x00fff000) >> 12;

	//Reset GPUSTAT Flag to receive next GP1 command
	gp1_ResetStatus();

	return true;
}

bool GPU::gp1_VDisplayRange()
{
	//GP1(07h)
	// Vertical Dysplay Range on Screen (gp1Data.0-9 = Y1, gp1Data.10-19 = Y2)
	 
	displayArea.y1 = gp1DataLatch & 0x000003ff;
	displayArea.y2 = (gp1DataLatch & 0x000ffc00) >> 10;

	//Reset GPUSTAT Flag to receive next GP1 command
	gp1_ResetStatus();

	return true;
}

bool GPU::gp1_DisplayMode()
{	
	//GP1(08h)
	//Modify various GPUSTAT bits

	uint32_t data;

	//GPUSTAT.17-18 << GP1DATA.0-1								Horizontal Resolution 1: 0 = 256, 1 = 320, 2 = 512, 3 = 640 					
	data = gp1DataLatch & 0x00000003;							//Extract bit value from GP1 Command
	gpuStat = (gpuStat & ~(0x00000003 << 17)) | (data << 17);	//Set GPUSTAT.17-18 to data value
	horizontalResolution1 = data;

	//GPUSTAT.19 << GP1DATA.2									Vertical Resolution: 0 = 240, 1 = 480
	data = (gp1DataLatch & 0x00000004) >> 2;					//Extract bit value from GP1 Command
	gpuStat = (gpuStat & ~(0x00000001 << 19)) | (data << 19);	//Set GPUSTAT.19 to data value
	verticalResolution = data;

	//GPUSTAT.20 << GP1DATA.3									Video Mode: 0 = NTSC, 1 = PAL
	data = (gp1DataLatch & 0x00000008) >> 3;					//Extract bit value from GP1 Command
	gpuStat = (gpuStat & ~(0x00000001 << 20)) | (data << 20);	//Set GPUSTAT.20 to data value
	videoMode = static_cast<VideoMode>(data);

	//GPUSTAT.21 << GP1DATA.4									Display Area Color Depth: 0 = 15bit, 1 = 24bit
	data = (gp1DataLatch & 0x00000010) >> 4;					//Extract bit value from GP1 Command
	gpuStat = (gpuStat & ~(0x00000001 << 21)) | (data << 21);	//Set GPUSTAT.21 to data value

	//GPUSTAT.22 << GP1DATA.5									Vertical Interlace: 0 = off, 1 = on
	data = (gp1DataLatch & 0x00000020) >> 5;					//Extract bit value from GP1 Command
	gpuStat = (gpuStat & ~(0x00000001 << 22)) | (data << 22);	//Set GPUSTAT.22 to data value
	verticalInterlace = static_cast<bool>(data);

	//GPUSTAT.16 << GP1DATA.6									Horizontal Resolution 2: 0 = 256/320/512/640, 1 = 368
	data = (gp1DataLatch & 0x00000040) >> 6;					//Extract bit value from GP1 Command
	gpuStat = (gpuStat & ~(0x00000001 << 16)) | (data << 16);	//Set GPUSTAT.16 to data value
	horizontalResolution2 = data;
	//GPUSTAT.14 << GP1DATA.7									//Reverse Flag: 0 = normal, 1 = distorted
	data = (gp1DataLatch & 0x00000080) >> 7;					//Extract bit value from GP1 Command
	gpuStat = (gpuStat & ~(0x00000001 << 14)) | (data << 14);	//Set GPUSTAT.16 to data value

	//Set DotClock Divider according to new Horizontal Resolution configuration
	if (horizontalResolution2 == 1)
	{
		dotClockRatio = 7; //H Res = 368
	}
	else if (horizontalResolution2 == 0)
	{
		switch (horizontalResolution1)
		{
		case 0:
			dotClockRatio = 10;	//H Res = 256
			break;
		case 1:
			dotClockRatio = 8;	//H Res = 320
			break;
		case 2:
			dotClockRatio = 5;	//H Res = 512
			break;
		case 3:
			dotClockRatio = 4;	//H Res = 640
			break;
		}
	}

	//Reset GPUSTAT Flag to receive next GP1 command
	gp1_ResetStatus();

	return true;
}

bool GPU::gp1_TextureDisable()
{
	//GP1(09h)
	//New Texture Disable (0 = normal, 1 = allow disable via GP0(E1h)

	uint32_t data;

	//GPUSTAT.15 << GP1DATA.0
	data = (gp1DataLatch & 0x00000001);							//Extract bit value from GP1 Command
	gpuStat = (gpuStat & ~(0x00000001 << 15)) | (data << 15);	//Set GPUSTAT.15 to data value

	//Reset GPUSTAT Flag to receive next GP1 command
	gp1_ResetStatus();

	return true;
}

bool GPU::gp1_Unknown()
{
	//printf("Unknown GP1 Command!\n");
	
	//Reset GPUSTAT Flag to receive next GP1 command
	gp1_ResetStatus();

	return false;
}

bool GPU::gp1_GetGpuInfo()
{
	//GP1(10h)
	//00h - 01h = Returns Nothing(old value in GPUREAD remains unchanged)
	//02h = Read Texture Window setting; GP0(E2h); 20bit / MSBs = Nothing
	//03h = Read Draw area top left; GP0(E3h); 19bit / MSBs = Nothing
	//04h = Read Draw area bottom right; GP0(E4h); 19bit / MSBs = Nothing
	//05h = Read Draw offset; GP0(E5h); 22bit
	//06h - 07h = Returns Nothing(old value in GPUREAD remains unchanged)
	//08h - FFFFFFh = Mirrors of 00h..07h
	//07h = Read GPU Type(usually 2); see "GPU Versions" chapter
	//08h = Unknown(Returns 00000000h) (lightgun on some GPUs ? )
	//09h - 0Fh = Returns Nothing(old value in GPUREAD remains unchanged)

	uint32_t data;

	data = gp1DataLatch & 0x00000007;
	switch (data)
	{
	case 2:
		gpuReadLatch = textureMask.x | textureMask.y << 5 | textureOffset.x << 10 | textureOffset.y << 15;
		break;
	case 3:
		gpuReadLatch = drawingArea.x1 | drawingArea.y1 << 10;
		break;
	case 4:
		gpuReadLatch = drawingArea.x2 | drawingArea.y2 << 10;
		break;
	case 5:
		gpuReadLatch = drawingOffset.x | drawingOffset.y << 11;
		break;
	case 7:
		gpuReadLatch = 0x00000002;	//GPU Version
		break;
	case 8:
		gpuReadLatch = 0x00000000;
	}

	//Reset GPUSTAT Flag to receive next GP1 command
	gp1_ResetStatus();

	return true;
}

void GPU::gp1_ResetStatus()
{
	recvCommand = false;
	recvParameters = false;
	gp1CommandAvailable = false;
}

//-----------------------------------------------------------------------------------------------------
// 
//                               DEBUG Status
// 
//-----------------------------------------------------------------------------------------------------
void GPU::getDebugInfo(GpuDebugInfo& info)
{
	info.gpuStat = gpuStat;
	info.vRam = static_cast<void*>(vRam);
	info.displayArea = displayArea;
	info.displayOffset = displayOffset;
	info.drawingArea = drawingArea;
	info.drawingOffset = drawingOffset;

	switch (videoMode)
	{
	case VideoMode::NTSC:
		info.videoStandard = "NTSC";

		if (verticalResolution == 0)
			info.videoResolution.y = 240;
		else if (verticalResolution == 1)
			info.videoResolution.y = 480;

		break;
	case VideoMode::PAL:
		info.videoStandard = "PAL";

		if (verticalResolution == 0)
			info.videoResolution.y = 256;
		else if (verticalResolution == 1)
			info.videoResolution.y = 512;

		break;
	}

	if (horizontalResolution2 == 1)
	{
		info.videoResolution.x = 368;
	}
	else if (horizontalResolution2 == 0)
	{
		switch (horizontalResolution1)
		{
		case 0:
			info.videoResolution.x = 256;
			break;
		case 1:
			info.videoResolution.x = 320;
			break;
		case 2:
			info.videoResolution.x = 512;
			break;
		case 3:
			info.videoResolution.x = 640;
			break;
		}
	}
}