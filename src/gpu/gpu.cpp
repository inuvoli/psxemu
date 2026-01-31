#include <loguru.hpp>
#include <array>
#include <cmath>
#include <algorithm>

#include "psx.h"
#include "renderer.h"
#include "gpu.h"

GPU::GPU()
{
	//Reset Scheduler Parameters
	schedulerClockRatio = 11.0f / 7.0f; //GPU Clock is 11/7 of CPU Clock
	schedulerClockTicks = 0.0f;
	
	//Init GPU Internal Registers
	gp0DataLatch = 0x00000000;
	gp1DataLatch = 0x00000000;
	gpuReadLatch = 0x00000000;
	gpuStat = 0x14802000;
	fifo.flush();
	
	//Reset Internal Flags
	hBlank = false;
	vBlank = false;

	hCount = 0x00;
	vCount = 0x00;

	displayMode = VideoMode::NTSC;
	displayDisabled = true;
	
	dotClockRatio = 10;				//Assuming 256 pixel per line is standard configuration
	tickCountPerScanline = NTSC_GPU_CLOCK_PER_SCANLINE;
	tickCountPerDots = tickCountPerScanline / dotClockRatio;
	tickCountPerHBlank = NTSC_GPU_CLOCK_PER_HBLANK;
	scanlinePerFrame = NTSC_SCANLINES_PER_FRAME;
	scanlinePerVBlank = NTSC_SCANLINES_PER_VBLANK;
	verticalInterlace = false;
	newScanline = false;
	newFrame = false;
	textureDisabled = false;
	colorMode = 0;
	semiTransparencyMode = 0;
	rectangleTexFlipX = false;
	rectangleTexFlipX = false;

	//VRAM & Video Settings
	memset(&displayResolution, 0, sizeof(lite::vec2t<uint16_t>));
	memset(&displayRange, 0, sizeof(lite::vec4t<uint16_t>));
	memset(&displayStart, 0, sizeof(lite::vec2t<uint16_t>));
	memset(&drawingArea, 0, sizeof(lite::vec4t<uint16_t>));
	memset(&drawingOffset, 0, sizeof(lite::vec2t<uint16_t>));
	memset(&textureMask, 0, sizeof(lite::vec2t<uint8_t>));
	memset(&textureOffset, 0, sizeof(lite::vec2t<uint8_t>));
	memset(&texturePage, 0, sizeof(lite::vec2t<uint16_t>));

	//Reset Internal Clock Counter
	gpuClockTicks = 0;
	
	//GPU Internal Status & Configurations
	gp0CommandAvailable = false;
	gp0Opcode = 0x00;
	gp0Command = 0x00000000;
	gp0CommandParameters = 0;
	gp0ReadParameters = 0;
	gp0CommandFifo = false;
	gp0RecvPolyLine = false;
	gp1CommandAvailable = false;
	gp1Command = 0x00000000;
	
	//ReceiveCommand Status
	recvCommand = false;
	recvParameters = false;

	//Memory Transfer Status & Configuration
	dataReadActive = false;
	dataWriteActive = false;
	memset(&dataDestination, 0, sizeof(lite::vec2t<uint16_t>));
	memset(&dataSource, 0, sizeof(lite::vec2t<uint16_t>));
	memset(&dataSize, 0, sizeof(lite::vec2t<uint16_t>));
	memset(&dataPointer, 0, sizeof(lite::vec2t<uint16_t>));

	//Init Renderer
	Renderer::Init();

	//Init GPU Instruction Dictionaries
	gp0InstrSet =
	{
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"Clear Texture Cache", &GPU::gp0_ClearTextureCache, 0, false, false},
		{"Fill Rectangle in VRAM", &GPU::gp0_FillVRam, 2, true, false},
		{"Unknown (NOP?)", &GPU::gp0_NoOperation, 0, true, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"Interrupt Request (IRQ1)", &GPU::gp0_InterruptRequest, 0, false, false},
		{"Monocrome 3 Point Polygon, Opaque", &GPU::gp0_Polygons, 3, true, true},
		{"Monocrome 3 Point Polygon, Opaque", &GPU::gp0_Polygons, 3, true, true},
		{"Monocrome 3 Point Polygon, Semi-Transparent", &GPU::gp0_Polygons, 3, true, true},
		{"Monocrome 3 Point Polygon, Semi-Transparent", &GPU::gp0_Polygons, 3, true, true},
		{"Textured 3 Point polygon, opaque, texture blending", &GPU::gp0_Polygons, 6, true, true},
		{"Textured 3 Point polygon, opaque, raw texture", &GPU::gp0_Polygons, 6, true, true},
		{"Textured 3 Point polygon, semi-transparent, texture blending", &GPU::gp0_Polygons, 6, true, true},
		{"Textured 3 Point polygon, semi-transparent, raw texture", &GPU::gp0_Polygons, 6, true, true},
		{"Monocrome 4 Point Polygon, Opaque", &GPU::gp0_Polygons, 4, true, true},
		{"Monocrome 4 Point Polygon, Opaque", &GPU::gp0_Polygons, 4, true, true},
		{"Monocrome 4 Point Polygon, Semi-Transparent", &GPU::gp0_Polygons, 4, true, true},
		{"Monocrome 4 Point Polygon, Semi-Transparent", &GPU::gp0_Polygons, 4, true, true},
		{"Textured 4 Point polygon, opaque, texture blending", &GPU::gp0_Polygons, 8, true, true},
		{"Textured 4 Point polygon, opaque, raw texture", &GPU::gp0_Polygons, 8, true, true},
		{"Textured 4 Point polygon, semi-transparent, texture blending", &GPU::gp0_Polygons, 8, true, true},
		{"Textured 4 Point polygon, semi-transparent, raw texture", &GPU::gp0_Polygons, 8, true, true},
		{"Shaded 3 Point polygon, opaque", &GPU::gp0_Polygons, 5, true, true},
		{"Shaded 3 Point polygon, opaque", &GPU::gp0_Polygons, 5, true, true},
		{"Shaded 3 Point polygon, semi-transparent", &GPU::gp0_Polygons, 5, true, true},
		{"Shaded 3 Point polygon, semi-transparent", &GPU::gp0_Polygons, 5, true, true},
		{"Shaded Textured 3 Point polygon, opaque, texture blending", &GPU::gp0_Polygons, 8, true, true},
		{"Shaded Textured 3 Point polygon, opaque, texture blending", &GPU::gp0_Polygons, 8, true, true},
		{"Shaded Textured 3 Point polygon, semi-transparent, texture blending", &GPU::gp0_Polygons, 8, true, true},
		{"Shaded Textured 3 Point polygon, semi-transparent, texture blending", &GPU::gp0_Polygons, 8, true, true},
		{"Shaded 4 Point polygon, opaque", &GPU::gp0_Polygons, 7, true, true},
		{"Shaded 4 Point polygon, opaque", &GPU::gp0_Polygons, 7, true, true},
		{"Shaded 4 Point polygon, semi-transparent", &GPU::gp0_Polygons, 7, true, true},
		{"Shaded 4 Point polygon, semi-transparent", &GPU::gp0_Polygons, 7, true, true},
		{"Shaded Textured 4 Point polygon, opaque, texture blending", &GPU::gp0_Polygons, 11, true, true},
		{"Shaded Textured 4 Point polygon, opaque, texture blending", &GPU::gp0_Polygons, 11, true, true},
		{"Shaded Textured 4 Point polygon, semi-transparent, texture blending", &GPU::gp0_Polygons, 11, true, true},
		{"Shaded Textured 4 Point polygon, semi-transparent, texture blending", &GPU::gp0_Polygons, 11, true, true},
		{"Monocrome line, opaque", &GPU::gp0_Lines, 2, true, true},
		{"", &GPU::gp0_NoOperation, 0, false, false},
		{"Monocrome line, semi-transparent", &GPU::gp0_Lines, 2, true, true},
		{"", &GPU::gp0_NoOperation, 0, false, false},
		{"", &GPU::gp0_NoOperation, 0, false, false},
		{"", &GPU::gp0_NoOperation, 0, false, false},
		{"", &GPU::gp0_NoOperation, 0, false, false},
		{"", &GPU::gp0_NoOperation, 0, false, false},
		{"Monocrome Polyline, opaque", &GPU::gp0_Lines, 255, true, true},
		{"", &GPU::gp0_NoOperation, 0, false, false},
		{"Monocrome Polyline, semi-transparent", &GPU::gp0_Lines, 255, true, true},
		{"", &GPU::gp0_NoOperation, 0, false, false},
		{"", &GPU::gp0_NoOperation, 0, false, false},
		{"", &GPU::gp0_NoOperation, 0, false, false},
		{"", &GPU::gp0_NoOperation, 0, false, false},
		{"", &GPU::gp0_NoOperation, 0, false, false},
		{"Shaded line, opaque", &GPU::gp0_Lines, 3, true, true},
		{"", &GPU::gp0_NoOperation, 0, false, false},
		{"Shaded line, semi-transparent", &GPU::gp0_Lines, 3, true, true},
		{"", &GPU::gp0_NoOperation, 0, false, false},
		{"", &GPU::gp0_NoOperation, 0, false, false},
		{"", &GPU::gp0_NoOperation, 0, false, false},
		{"", &GPU::gp0_NoOperation, 0, false, false},
		{"", &GPU::gp0_NoOperation, 0, false, false},
		{"Shaded Polyline, opaque", &GPU::gp0_Lines, 255, true, true},
		{"", &GPU::gp0_NoOperation, 0, false, false},
		{"Shaded Polyline. semi-transparent", &GPU::gp0_Lines, 255, true, true},
		{"", &GPU::gp0_NoOperation, 0, false, false},
		{"", &GPU::gp0_NoOperation, 0, false, false},
		{"", &GPU::gp0_NoOperation, 0, false, false},
		{"", &GPU::gp0_NoOperation, 0, false, false},
		{"", &GPU::gp0_NoOperation, 0, false, false},
		{"Monocrome Rectangle, variable size, opaque", &GPU::gp0_Rectangles, 2, true, false},
		{"", &GPU::gp0_NoOperation, 0, false, false},
		{"Monocrome Rectangle, variable size, semi-transparent", &GPU::gp0_Rectangles, 2, true, false},
		{"", &GPU::gp0_NoOperation, 0, false, false},
		{"Textured Rectangle, variable size, opaque, texture blending", &GPU::gp0_Rectangles, 3, true, false},
		{"Textured Rectangle, variable size, opaque, raw texture", &GPU::gp0_Rectangles, 3, true, false},
		{"Textured Rectangle, variable size, semi-transparent, texture blending", &GPU::gp0_Rectangles, 3, true, false},
		{"Textured Rectangle, variable size, semi-transparent, raw texture", &GPU::gp0_Rectangles, 3, true, false},
		{"Monocrome Rectangle, 1x1, opaque", &GPU::gp0_Rectangles, 1, true, false},
		{"", &GPU::gp0_NoOperation, 0, false, false},
		{"Monocrome Rectangle, 1x1, semi-transparent", &GPU::gp0_Rectangles, 1, true, false},
		{"", &GPU::gp0_NoOperation, 0, false, false},
		{"Textured Rectangle, 1x1, opaque, texture blending", &GPU::gp0_Rectangles, 2, true, false},
		{"Textured Rectangle, 1x1, opaque, raw texture", &GPU::gp0_Rectangles, 2, true, false},
		{"Textured Rectangle, 1x1, semi-transparent, texture blending", &GPU::gp0_Rectangles, 2, true, false},
		{"Textured Rectangle, 1x1, semi-transparent, raw texture", &GPU::gp0_Rectangles, 2, true, false},
		{"Monocrome Rectangle, 8x8, opaque", &GPU::gp0_Rectangles, 1, true, false},
		{"", &GPU::gp0_NoOperation, 0, false, false},
		{"Monocrome Rectangle, 8x8, semi-transparent", &GPU::gp0_Rectangles, 1, true, false},
		{"", &GPU::gp0_NoOperation, 0, false, false},
		{"Textured Rectangle, 8x8, opaque, texture blending", &GPU::gp0_Rectangles, 2, true, false},
		{"Textured Rectangle, 8x8, opaque, raw texture", &GPU::gp0_Rectangles, 2, true, false},
		{"Textured Rectangle, 8x8, semi-transparent, texture blending", &GPU::gp0_Rectangles, 2, true, false},
		{"Textured Rectangle, 8x8, semi-transparent, raw texture", &GPU::gp0_Rectangles, 2, true, false},
		{"Monocrome Rectangle, 16x16, opaque", &GPU::gp0_Rectangles, 1, true, false},
		{"", &GPU::gp0_NoOperation, 0, false, false},
		{"Monocrome Rectangle, 16x16, semi-transparent", &GPU::gp0_Rectangles, 1, true, false},
		{"", &GPU::gp0_NoOperation, 0, false, false},
		{"Textured Rectangle, 16x16, opaque, texture blending", &GPU::gp0_Rectangles, 2, true, false},
		{"Textured Rectangle, 16x16, opaque, raw texture", &GPU::gp0_Rectangles, 2, true, false},
		{"Textured Rectangle, 16x16, semi-transparent, texture blending", &GPU::gp0_Rectangles, 2, true, false},
		{"Textured Rectangle, 16x16, semi-transparent, raw texture", &GPU::gp0_Rectangles, 2, true, false},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_CopyVRam2VRam, 3, true, false},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_CopyVRam2VRam, 3, true, false},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_CopyVRam2VRam, 3, true, false},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_CopyVRam2VRam, 3, true, false},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_CopyVRam2VRam, 3, true, false},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_CopyVRam2VRam, 3, true, false},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_CopyVRam2VRam, 3, true, false},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_CopyVRam2VRam, 3, true, false},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_CopyVRam2VRam, 3, true, false},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_CopyVRam2VRam, 3, true, false},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_CopyVRam2VRam, 3, true, false},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_CopyVRam2VRam, 3, true, false},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_CopyVRam2VRam, 3, true, false},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_CopyVRam2VRam, 3, true, false},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_CopyVRam2VRam, 3, true, false},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_CopyVRam2VRam, 3, true, false},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_CopyVRam2VRam, 3, true, false},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_CopyVRam2VRam, 3, true, false},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_CopyVRam2VRam, 3, true, false},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_CopyVRam2VRam, 3, true, false},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_CopyVRam2VRam, 3, true, false},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_CopyVRam2VRam, 3, true, false},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_CopyVRam2VRam, 3, true, false},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_CopyVRam2VRam, 3, true, false},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_CopyVRam2VRam, 3, true, false},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_CopyVRam2VRam, 3, true, false},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_CopyVRam2VRam, 3, true, false},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_CopyVRam2VRam, 3, true, false},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_CopyVRam2VRam, 3, true, false},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_CopyVRam2VRam, 3, true, false},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_CopyVRam2VRam, 3, true, false},
		{"Copy Rectangle (VRAM to VRAM)", &GPU::gp0_CopyVRam2VRam, 3, true, false},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_CopyRam2VRam, 2, true, false},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_CopyRam2VRam, 2, true, false},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_CopyRam2VRam, 2, true, false},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_CopyRam2VRam, 2, true, false},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_CopyRam2VRam, 2, true, false},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_CopyRam2VRam, 2, true, false},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_CopyRam2VRam, 2, true, false},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_CopyRam2VRam, 2, true, false},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_CopyRam2VRam, 2, true, false},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_CopyRam2VRam, 2, true, false},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_CopyRam2VRam, 2, true, false},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_CopyRam2VRam, 2, true, false},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_CopyRam2VRam, 2, true, false},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_CopyRam2VRam, 2, true, false},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_CopyRam2VRam, 2, true, false},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_CopyRam2VRam, 2, true, false},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_CopyRam2VRam, 2, true, false},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_CopyRam2VRam, 2, true, false},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_CopyRam2VRam, 2, true, false},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_CopyRam2VRam, 2, true, false},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_CopyRam2VRam, 2, true, false},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_CopyRam2VRam, 2, true, false},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_CopyRam2VRam, 2, true, false},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_CopyRam2VRam, 2, true, false},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_CopyRam2VRam, 2, true, false},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_CopyRam2VRam, 2, true, false},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_CopyRam2VRam, 2, true, false},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_CopyRam2VRam, 2, true, false},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_CopyRam2VRam, 2, true, false},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_CopyRam2VRam, 2, true, false},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_CopyRam2VRam, 2, true, false},
		{"Copy Rectangle (RAM to VRAM)", &GPU::gp0_CopyRam2VRam, 2, true, false},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_CopyVRam2Ram, 2, true, false},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_CopyVRam2Ram, 2, true, false},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_CopyVRam2Ram, 2, true, false},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_CopyVRam2Ram, 2, true, false},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_CopyVRam2Ram, 2, true, false},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_CopyVRam2Ram, 2, true, false},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_CopyVRam2Ram, 2, true, false},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_CopyVRam2Ram, 2, true, false},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_CopyVRam2Ram, 2, true, false},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_CopyVRam2Ram, 2, true, false},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_CopyVRam2Ram, 2, true, false},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_CopyVRam2Ram, 2, true, false},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_CopyVRam2Ram, 2, true, false},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_CopyVRam2Ram, 2, true, false},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_CopyVRam2Ram, 2, true, false},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_CopyVRam2Ram, 2, true, false},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_CopyVRam2Ram, 2, true, false},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_CopyVRam2Ram, 2, true, false},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_CopyVRam2Ram, 2, true, false},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_CopyVRam2Ram, 2, true, false},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_CopyVRam2Ram, 2, true, false},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_CopyVRam2Ram, 2, true, false},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_CopyVRam2Ram, 2, true, false},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_CopyVRam2Ram, 2, true, false},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_CopyVRam2Ram, 2, true, false},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_CopyVRam2Ram, 2, true, false},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_CopyVRam2Ram, 2, true, false},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_CopyVRam2Ram, 2, true, false},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_CopyVRam2Ram, 2, true, false},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_CopyVRam2Ram, 2, true, false},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_CopyVRam2Ram, 2, true, false},
		{"Copy Rectangle (VRAM to RAM)", &GPU::gp0_CopyVRam2Ram, 2, true, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"Draw Mode Setting", &GPU::gp0_DrawMode, 0, false, false},
		{"Texture Window Setting", &GPU::gp0_TextureSetting, 0, false, false},
		{"Set Drawing Area Top Left", &GPU::gp0_SetDrawAreaTop, 0, false, false},
		{"Set Drawing Area Bottom Right", &GPU::gp0_SetDrawAreaBottom, 0, false, false},
		{"Set Drawing Offset", &GPU::gp0_SetDrawOffset, 0, false, false},
		{"Mask Bit Setting", &GPU::gp0_SetMaskBit, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false},
		{"NOP", &GPU::gp0_NoOperation, 0, false, false}
	};

	gp1InstrSet =
	{
		{"Reset GPU", &GPU::gp1_ResetGpu, 0},
		{"Reset Command Buffer", &GPU::gp1_ResetFifo, 0},
		{"Ack GPU Interrupt", &GPU::gp1_AckInterrupt, 0},
		{"Display Enable", &GPU::gp1_DisplayEnable, 1},
		{"DMA Direction", &GPU::gp1_DmaDirection, 1},
		{"Start Display Area VRAM", &GPU::gp1_StartDisplayArea, 2},
		{"Horizontal Display Range", &GPU::gp1_HDisplayRange, 2},
		{"Vertical Display Range", &GPU::gp1_VDisplayRange, 2},
		{"Display Mode", &GPU::gp1_DisplayMode, 7},
		{"New Texture Disable", &GPU::gp1_NewTextureDisable, 1},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Get GPU Info", &GPU::gp1_GetGpuInfo, 1},
		{"Get GPU Info", &GPU::gp1_GetGpuInfo, 1},
		{"Get GPU Info", &GPU::gp1_GetGpuInfo, 1},
		{"Get GPU Info", &GPU::gp1_GetGpuInfo, 1},
		{"Get GPU Info", &GPU::gp1_GetGpuInfo, 1},
		{"Get GPU Info", &GPU::gp1_GetGpuInfo, 1},
		{"Get GPU Info", &GPU::gp1_GetGpuInfo, 1},
		{"Get GPU Info", &GPU::gp1_GetGpuInfo, 1},
		{"Get GPU Info", &GPU::gp1_GetGpuInfo, 1},
		{"Get GPU Info", &GPU::gp1_GetGpuInfo, 1},
		{"Get GPU Info", &GPU::gp1_GetGpuInfo, 1},
		{"Get GPU Info", &GPU::gp1_GetGpuInfo, 1},
		{"Get GPU Info", &GPU::gp1_GetGpuInfo, 1},
		{"Get GPU Info", &GPU::gp1_GetGpuInfo, 1},
		{"Get GPU Info", &GPU::gp1_GetGpuInfo, 1},
		{"Get GPU Info", &GPU::gp1_GetGpuInfo, 1},
		{"Special Texture Disable", &GPU::gp1_NewTextureDisable, 1},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Unknown", &GPU::gp1_Unknown, 0},
		{"Unknown", &GPU::gp1_Unknown, 0}
	};
}

GPU::~GPU()
{

}

bool GPU::reset()
{
	//Reset Scheduler Parameters
	schedulerClockRatio = 11.0f / 7.0f; //GPU Clock is 11/7 of CPU Clock
	schedulerClockTicks = 0.0f;

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

	displayMode = VideoMode::NTSC;
	dotClockRatio = 10;				//Assuming 256 pixel per line is standard configuration
	tickCountPerScanline = NTSC_GPU_CLOCK_PER_SCANLINE;
	tickCountPerDots = tickCountPerScanline / dotClockRatio;
	tickCountPerHBlank = NTSC_GPU_CLOCK_PER_HBLANK;
	scanlinePerFrame = NTSC_SCANLINES_PER_FRAME;
	scanlinePerVBlank = NTSC_SCANLINES_PER_VBLANK;
	verticalInterlace = false;
	newScanline = false;
	newFrame = false;
	textureDisabled = false;
	colorMode = 0;
	semiTransparencyMode = 0;
	rectangleTexFlipX = false;
	rectangleTexFlipX = false;

	//Video Settings
	memset(&displayResolution, 0, sizeof(lite::vec2t<uint16_t>));
	memset(&displayRange, 0, sizeof(lite::vec4t<uint16_t>));
	memset(&displayStart, 0, sizeof(lite::vec2t<uint16_t>));
	memset(&drawingArea, 0, sizeof(lite::vec4t<uint16_t>));
	memset(&drawingOffset, 0, sizeof(lite::vec2t<uint16_t>));
	memset(&textureMask, 0, sizeof(lite::vec2t<uint8_t>));
	memset(&textureOffset, 0, sizeof(lite::vec2t<uint8_t>));
	memset(&texturePage, 0, sizeof(lite::vec2t<uint16_t>));

	//Reset Internal Clock Counter
	gpuClockTicks = 0;
	
	//GPU Internal Status & Configurations
	gp0CommandAvailable = false;
	gp0Opcode = 0x00;
	gp0Command = 0x00000000;
	gp0CommandParameters = 0;
	gp0ReadParameters = 0;
	gp0CommandFifo = false;
	gp0RecvPolyLine = false;
	gp1CommandAvailable = false;
	gp1Command = 0x00000000;
	
	//ReceiveCommand Status
	recvCommand = false;
	recvParameters = false;

	//Memory Transfer Status & Configuration
	dataReadActive = false;
	dataWriteActive = false;
	memset(&dataDestination, 0, sizeof(lite::vec2t<uint16_t>));
	memset(&dataSource, 0, sizeof(lite::vec2t<uint16_t>));
	memset(&dataSize, 0, sizeof(lite::vec2t<uint16_t>));
	memset(&dataPointer, 0, sizeof(lite::vec2t<uint16_t>));

	//Reset Renderer Status
	Renderer::Reset();

	return true;
}

//-----------------------------------------------------------------------------------------------------
// 
//                               HELPER FUNCTIONS
// 
//-----------------------------------------------------------------------------------------------------
void GPU::writeVRAM(uint32_t& data)
{
	uint16_t h, l, x1, x2, y1, y2;

	y1 = dataPointer.y % 512;
	y2 = y1;
	x1 = dataPointer.x % 1024;
	x2 = (dataPointer.x + 1) % 1024;

	l = static_cast<uint16_t>((data & 0x0000ffff));
	h = static_cast<uint16_t>((data & 0xffff0000) >> 16);

	Renderer::WriteVRAM(x1, y1, l);
	Renderer::WriteVRAM(x2, y2, h);
	
	//Update Data Pointer in VRAM
	dataPointer.x += 2;
	if (dataPointer.x >= dataDestination.x + dataSize.x)
	{
		dataPointer.x = dataDestination.x;
		dataPointer.y++;
	}

	if (dataPointer.y >= dataDestination.y + dataSize.y)
	{
		dataWriteActive = false;
		Renderer::CommitVRAMWrite();
	}
		

	return;
}

uint32_t GPU::readVRAM()
{
	uint32_t data;
	uint16_t h, l, x1, x2, y1, y2;

	y1 = dataPointer.y % 512;
	y2 = y1;
	x1 = dataPointer.x % 1024;
	x2 = (dataPointer.x + 1) % 1024;

	l = Renderer::ReadVRAM(x1, y1);
	h = Renderer::ReadVRAM(x2, y2);

	data = (h << 16) | l;

	//Update Data Pointer in VRAM
	dataPointer.x += 2;
	if (dataPointer.x >= dataSource.x + dataSize.x)
	{
		dataPointer.x = dataSource.x;
		dataPointer.y++;
	}

	if (dataPointer.y >= dataSource.y + dataSize.y)
		dataReadActive = false;

	return data;
}

bool GPU::updateVHBlank()
{
	bool currentVBlank = vBlank;
	bool currentHBlank = hBlank;

	//Reset NewFrame and NewScanline Status
	newScanline = false;
	newFrame = false;

	hCount++;
	if (hCount == tickCountPerScanline)
	{
		hCount = 0;
		vCount++;
		newScanline = true;
	}
	if (vCount == scanlinePerFrame)
	{
		vCount = 0;
		newFrame = true;
	}

	//Update hBlank and vBlank Status
	hBlank = !(hCount < (tickCountPerScanline - tickCountPerHBlank));
	vBlank = !(vCount < (scanlinePerFrame - scanlinePerVBlank));

	//Check if I need to trigger vBlank Interrupt or hBlank Clock on raising edge
	//According to vBlank and hBlank new status
	if (!currentVBlank && vBlank)
	{
		//Raising Edge of vBlank
		psx->interrupt->request(static_cast<uint32_t>(interrupt::Cause::vblank));
		//Trigger also the renderer in order to present the new frame every vBlank
		Renderer::vBlank();
	}

	if (!currentHBlank && hBlank)
	{
		//Raising Edge of hBlank
		psx->timers->execute(ClockSource::hBlank);
	}

	return true;
}

//-----------------------------------------------------------------------------------------------------
// 
//                               GPU Interface
// 
//-----------------------------------------------------------------------------------------------------
bool GPU::runticks()
{
	schedulerClockTicks += schedulerClockRatio;
	while (schedulerClockTicks >= 1.0f)
	{
		execute();
		schedulerClockTicks -= 1.0f;
	}
	return true;
}

bool GPU::execute()
{
	uint32_t data;

	auto gp0RunCommand = [&]()
	{
		bool bResult;

		if (gp0CommandFifo)
		{
			//It is a Command stored on the FIFO
			//Flush the Command from FIFO first. Already have its value on gp0Command.	
			uint32_t tmp;
			fifo.pop(tmp);
			
			LOG_F(1, "GPU - %s (params: %d)", gp0InstrSet[gp0Opcode].mnemonic.c_str(), gp0InstrSet[gp0Opcode].parameters);
			bResult = (this->*gp0InstrSet[gp0Opcode].operate)();
			if (!bResult)
				LOG_F(ERROR, "GPU - Unimplemented GP0 Command %s!", gp0InstrSet[gp0Opcode].mnemonic.c_str());
		}
		else
		{
			//It isn't a command stored on the FIFO
			//Actual GP0 Command its already on gp0Command

			LOG_F(1, "GPU - %s (params: %d)", gp0InstrSet[gp0Opcode].mnemonic.c_str(), gp0InstrSet[gp0Opcode].parameters);
			bResult = (this->*gp0InstrSet[gp0Opcode].operate)();
			if (!bResult)
				LOG_F(ERROR, "GPU - Unimplemented GP0 Command %s!", gp0InstrSet[gp0Opcode].mnemonic.c_str()); 
		}
	
		return 0;
	};
	auto gp1RunCommand = [&]()
	{
		bool bResult;
		LOG_F(1, "GPU - %s (params: %d)", gp1InstrSet[gp1Opcode].mnemonic.c_str(), gp1InstrSet[gp1Opcode].parameters);
		bResult = (this->*gp1InstrSet[gp1Opcode].operate)();		//GP1 Command are always executed immediately
		if (!bResult)
				LOG_F(ERROR, "GPU - Unimplemented GP1 Command %s!", gp1InstrSet[gp1Opcode].mnemonic.c_str()); 
		return 0;
	};

	//Update Vertical and Horizontal Blank Flag and trigger Interrupts/Clocks
	updateVHBlank();

	//Update GPUSTAT.31 - Interlace Lines (0 = Drawing Even Lines or vBlank, 1 = Drawing Odd Line)
	//  - Toggle at every new scanline if GPUSTAT.19 = 0
	//  - Toggle at every new frame if GPUSTAT.19 = 1
	//  - Always Zero during vBlank
	if (vBlank) { gpuStat &= ~(1UL << 31); };
	if (newScanline && !static_cast<bool>((gpuStat & (1UL << 19)))) { gpuStat ^= (1UL << 31); };
	if (newFrame && static_cast<bool>((gpuStat & (1UL << 19)))) { gpuStat ^= (1UL << 31); };

	// Update GPUSTAT.29-30 - DMA Direction
	// Updated in gp1_DmaDirection (0=Off, 1=FIFO, 2=RAM to VRAM, 3=VRAM to RAM)
	
	//Update GPUSTAT.28
	//  - Set to 0 after receiving both GP0/1 Command and all GP0/1 Parameters (GPU is Busy)
	//  - Set to 0 immediately after receiving Polygons or Lines Command (GPU is Busy)
	//  - Set to 1 when GPU is ready to receive a new Command and Parameters (previous command completed execution)
	gpuStat = (gpuStat & ~(1UL << 28)) | (static_cast<uint32_t>(!((recvCommand && recvParameters) || gp0RecvPolyLine)) << 28);

	//Update GPUSTAT.27
	//  - Set to 1 by GP0(C0h) - DMA transfer from VRAM to RAM
	//  - Set to 0 when all data has been transferred to RAM
	gpuStat = (gpuStat & ~(1UL << 27)) | (static_cast<uint32_t>(dataReadActive) << 27);
	
	//Update GPUSTAT.26
	//  - Set to 0 after receiving a GP0 Command or when GPU is Busy
	//  - Set to 1 when GPU is ready to receive a new command (previous command completed execution)
	gpuStat = (gpuStat & ~(1UL << 26)) | (static_cast<uint32_t>(!recvCommand) << 26);

	//Update GPUSTAT.25
	// - When GPUSTAT.29-30 = 0 --->Always zero(0)
	// - When GPUSTAT.29-30 = 1 --->FIFO State(0 = Full, 1 = Not Full)
	// - When GPUSTAT.29-30 = 2 --->Same as GPUSTAT.28
	// - When GPUSTAT.29-30 = 3 --->Same as GPUSTAT.27
	uint32_t dmaDirection = (gpuStat >> 29) & 0x03;
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
	if (!(gpuClockTicks % tickCountPerDots))
	{
		psx->timers->execute(ClockSource::Dot);
	}
	gpuClockTicks++;

	return true;
}

bool GPU::writeAddr(uint32_t addr, uint32_t& data, uint8_t bytes)
{
	auto gp0ReceiveCommand = [&](uint32_t word)
	{
		if (!recvCommand)
		{
			recvCommand = true;
			gp0Opcode = word >> 24;
			gp0Command = word;						
			gp0CommandParameters = gp0InstrSet[gp0Opcode].parameters;
			gp0CommandFifo = gp0InstrSet[gp0Opcode].fifo;
			gp0RecvPolyLine = gp0InstrSet[gp0Opcode].polyline;
			gp0ReadParameters = 0;
			
			//If GP0 command uses FIFO, push the Command over it. It will be extracted when the command is executed.
			if (gp0CommandFifo)
				fifo.push(word);
			
			if (gp0CommandParameters == 0)
			{
				recvParameters = true;
				gp0CommandAvailable = true;
			}

			return 0;
		}

		if (!recvParameters)
		{
			fifo.push(word);
			gp0ReadParameters++;
			if (gp0ReadParameters == gp0CommandParameters)
			{
				recvParameters = true;
				gp0CommandAvailable = true;
			}
		}
				
		return 0;
	};
	auto gp1ReceiveCommand = [&](uint32_t word)
	{
		//All GP1 Commands don't have parameters and don't use FIFO
		recvCommand = true;
		recvParameters = true;
		gp1CommandAvailable = true;
		gp1Command = word;
		gp1Opcode = (gp1Command >> 24) & 0x3f;	//Extract Command Opcode, masked since from 0x40 are all mirrored
		
		return 0;
	};
	
	uint32_t dmaDirection;

	switch (addr)
	{
	case 0x1f801810:	//--------------------------GP0 Command
		gp0DataLatch = data;						//Rendering and VRAM Access
		
		dmaDirection = (gpuStat >> 29) & 0x3;
		if (dataWriteActive && (dmaDirection == 2 || dmaDirection == 0))
		{
			//dataWriteActive is set by GP0(A0h) - Copy Rectangle (CPU to VRAM)
			//DmaDirection [GPUSTAT.21-30] is set by GP1(04h) - DMA Direction
			//Data Transfer could be either thru:
			//	- CPU, GPUSTAT.29-30 (DMA Direction) = 0  "DMA Off"
			//	- DMA, GPUSTAT.29-30 (DMA Direction) = 2  "DMA RAM to VRAM"
			writeVRAM(data);
		}
		else
		{
			gp0ReceiveCommand(data);
		};
		break;
	case 0x1f801814:	//--------------------------GP1 Command
		gp1DataLatch = data;						//Display Control
		gp1ReceiveCommand(data);					//Receive GP1 Command and update GPUSTAT flags
		break;

	default:
		LOG_F(ERROR, "GPU - Unknown Parameter Set addr: 0x%08x (%d), data: 0x%08x", addr, bytes, data);
		return false;
	}

	LOG_F(3, "GPU - Write to Register:\t\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
	return true;
}

uint32_t GPU::readAddr(uint32_t addr, uint8_t bytes)
{
	uint32_t data;
	uint32_t dmaDirection;
	
	switch (addr)
	{
	case 0x1f801810:
		data = gpuReadLatch;	//--------------------------------------Response from GP0 and GP1 commands
		
		dmaDirection = (gpuStat >> 29) & 0x3;
		if (dataReadActive && (dmaDirection == 3 || dmaDirection == 0))
			//dataReadActive is set by GP0(C0h) - Copy Rectangle (VRAM to CPU)
			//DmaDirection [GPUSTAT.21-30] is set by GP1(04h) - DMA Direction
			//Data Transfer could be either thru:
			//	- CPU, GPUSTAT.29-30 (DMA Direction) = 0  "DMA Off"
			//	- DMA, GPUSTAT.29-30 (DMA Direction) = 3  "DMA VRAM to RAM"
			data = readVRAM();
		break;
	case 0x1f801814:
		data = gpuStat;			//--------------------------------------GPU Status Register
		break;

	default:
		LOG_F(ERROR, "GPU - Unknown Parameter Get addr: 0x%08x (%d)\n", addr, bytes);
		return 0x0;
	}

	LOG_F(3, "GPU - Read from Register:\t\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
	return data;
}
//-----------------------------------------------------------------------------------------------------
// 
//                               GP0 Full Instruction Set
// 
//-----------------------------------------------------------------------------------------------------

bool GPU::gp0_Lines()
{
	// Parse Lines Render Command
	// Bit Number   Value   Meaning
	// 7-5          010     Line Render
	// 4			1/0		gourad / flat shading
	// 3          	1/0     polyline / single line
	// 2            1/0     Unused
	// 1            1/0     semi-transparent / opaque
	// 0            1/0     Unused
	bool		shaded = (gp0Opcode & 0x10) ? true : false;
	bool		polyline  = (gp0Opcode & 0x08) ? true : false;
	bool		semiTransparent  = (gp0Opcode & 0x02) ? true : false;
	uint32_t	param;
		
	param = gp0Command;

	//Set Renderer Status Before Rendering Lines
	Renderer::SetTransparency(semiTransparent);
	
	//Set Vertex Buffer
	GpuVertex verts[2];

	//First Vertex
	decodeColor(param, verts[0]);
	fifo.pop(param);
	decodePosition(param, verts[0]);
	fifo.pop(param);

	if (polyline)
	{
		while((param & 0xf000f000) != 0x50005000)
		{
			//Other Vertex
			verts[1] = verts[0]; //Settink v1 color equal to v0 color for Monocrome Lines.
			if (shaded)
			{
				decodeColor(param, verts[1]);
				fifo.pop(param);
			}
			decodePosition(param, verts[1]);
			fifo.pop(param);

			//Render Line Segment
			Renderer::DrawLine(verts);

			//Shift vertex for next segment
			verts[0] = verts[1];
		}
	}
	else
	{
		//Second Vertex
		verts[1] = verts[0]; //Settink v1 color equal to v0 color for Monocrome Lines.
		if (shaded)
		{
			decodeColor(param, verts[1]);
			fifo.pop(param);
		}
		decodePosition(param, verts[1]);
		Renderer::DrawLine(verts);
	}	
	
	//Reset GPUSTAT Flag to receive next GP0 command
	gp0_ResetStatus();

	return true;
}

bool GPU::gp0_Rectangles()
{
	// Parse Rectangles Render Command
	// Bit Number   Value   Meaning
	// 7-5          011     Rectangle render
	// 4-3          ss     	Rectangle size (00 = variable, 01 = single pixel, 10 = 8x8 sprite, 11, 16x16 sprite
	// 2            1/0     textured / untextured
	// 1            1/0     semi-transparent / opaque
	// 0            1/0     raw texture / texture blending
	
	uint8_t		recType = (gp0Opcode & 0x18) >> 3;
	bool		textured  = (gp0Opcode & 0x04) ? true : false;
	bool		semiTransparent  = (gp0Opcode & 0x02) ? true : false;
	bool		texblending  = (gp0Opcode & 0x01) ? false : true;
	uint16_t	texPageInfo = 0;
	uint16_t	clutInfo = 0;

	uint32_t	param;
	GpuVertex	recDimension;

	// Parse all Polygon Command Parameters from FIFO directly into Triangle vertex temporaries
	param = gp0Command;

	LOG_F(2, "GPU - Rectangle Type: %d, Textured: %d, SemiTransparent: %d, TextureBlending: %d", recType, textured, semiTransparent, texblending);
	LOG_F(2, "GPU - Rectangle Parameter [0]: 0x%08x", param);
	for(int i = 0; i<fifo.length(); ++i)
	{
		LOG_F(2, "GPU - Rectangle Parameter [%d]: 0x%08x", i+1, fifo[i]);
	}

	//Set Vertex Buffer
	GpuVertex verts[MAX_VERTEX_NUMBER];
	
	//Extract All Rectangle Parameters
	decodeColor(param, verts[0]);
	fifo.pop(param);
	decodePosition(param, verts[0]);
	
	//Texture info, only for textured rectangles
	if (textured)										
	{
		fifo.pop(param);
		clutInfo = decodeTexture(param, verts[0]);
	}
	else
	{
		for(int i = 0; i < 4; ++i)
		{
			verts[i].u = 0;
			verts[i].v = 0;
		}
	}
	
	//Set Rectangle Size according to type
	switch (recType)
	{
		case 0:			//Variable Size
			fifo.pop(param);
			decodePosition(param, recDimension);
			break;
		case 1:			//Single pixel
			recDimension.x = 1;
			recDimension.y = 1;
			break;
		case 2:			//8x8 Sprite
			recDimension.x = 8;
			recDimension.y = 8;
			break;
		case 3:			//16x16 Sprite
			recDimension.x = 16;
			recDimension.y = 16;
			break;
	};

	//Extract CLUT Info if present
	lite::vec2t<uint16_t> clutCoords{0, 0};
	decodeClut(clutInfo, clutCoords);

	//Generate All remaining vertex
	verts[1] = verts[0];  //Start from first vertex values
	verts[1].x += recDimension.x;
	verts[2] = verts[0];  //Start from first vertex values
	verts[2].x += recDimension.x;
	verts[2].y += recDimension.y;
	verts[3] = verts[0];  //Start from first vertex values
	verts[3].y += recDimension.y;
	
	if (textured)
	{
		verts[1].u += recDimension.x;
		verts[2].u += recDimension.x;
		verts[2].v += recDimension.y;
		verts[3].v += recDimension.y;
	}

	//Set Renderer Status
	if (textured)
	{
		Renderer::SetTextureMode(textured, texblending);
		Renderer::SetTextureColorMode(colorMode);
		Renderer::SetClutTable(clutCoords);
	}
	Renderer::SetTransparency(semiTransparent);
	
	//Draw Rectangle
	if (recType == 1)
		Renderer::DrawPoint(verts);
	else
		Renderer::DrawRectangle(verts);
	
	//Reset GPUSTAT Flag to receive next GP0 command
	gp0_ResetStatus();

	return true;
}

bool GPU::gp0_Polygons()
{
	//Parse Polygon Render Command
	// Bit Number   Value   Meaning
	// 7-5          001     Polygon render
	// 4            1/0     gouraud / flat shading
	// 3            1/0     4 / 3 vertices
	// 2            1/0     textured / untextured
	// 1            1/0     semi transparent / solid
	// 0            1/0     raw texture / texture blending
	bool		shaded = (gp0Opcode & 0x10) ? true : false;
	bool		textured  = (gp0Opcode & 0x04) ? true : false;
	bool		semiTransparent  = (gp0Opcode & 0x02) ? true : false;
	bool		texblending  = (gp0Opcode & 0x01) ? false : true;
	uint8_t		vertexNum = (gp0Opcode & 0x08) ? 4 : 3;		
	
	uint16_t	texPageInfo = 0;
	uint16_t	clutInfo = 0;
	uint32_t 	param;
	
	// Parse all Polygon Command Parameters from FIFO directly into Triangle vertex temporaries
	param = gp0Command;
	
	//Set Vertex Buffer
	GpuVertex verts[MAX_VERTEX_NUMBER];

	//Extract all Vertex Data
	for (int i = 0; i < vertexNum; ++i)
	{
		for (int j = 0; j < MAX_POLYGON_PARAMS; ++j)
		{
			switch (j)
			{
			case 0: // COLOR
				if (shaded || i == 0)
				{
					decodeColor(param, verts[i]);
					fifo.pop(param);
				} // Shaded Polygons have a different colour for each vertex
				else
				{
					decodeColor(gp0Command, verts[i]);
				}  // Monocrome Polygons use first vertex color for all vertex
				break;

			case 1: // POSITION
				decodePosition(param, verts[i]);
				fifo.pop(param);
				break;
			case 2: // TEXTURE
				if (textured && i == 0) { clutInfo = decodeTexture(param, verts[i]); fifo.pop(param);}
				else if (textured && i == 1) { texPageInfo = decodeTexture(param, verts[i]); fifo.pop(param);}
				else if (textured && i > 1) { decodeTexture(param, verts[i]); fifo.pop(param);}
				else { verts[i].u = 0; verts[i].v = 0;}
				break;
			}
		}
	}
	
	if (textured)
	{
		//Extract Texture Page Info if present
		lite::vec2t<uint16_t> texPageCoords{0, 0};
		uint8_t colorMode = 0;
		uint8_t semiTransMode = 0;
		decodeTexPage(texPageInfo, texPageCoords, colorMode, semiTransMode);

		//Extract CLUT Info if present
		lite::vec2t<uint16_t> clutCoords{0, 0};
		decodeClut(clutInfo, clutCoords);

		Renderer::SetTransparencyMode(semiTransMode);	
		Renderer::SetTextureColorMode(colorMode);
		Renderer::SetTexturePage(texPageCoords);
		Renderer::SetClutTable(clutCoords);
	}
	
	//Set Renderer Status
	Renderer::SetTransparency(semiTransparent);
	Renderer::SetTextureMode(textured, texblending);
	
	//Draw Polygon depending on number of vertices
	Renderer::DrawPolygon(verts, vertexNum);

	//Reset GPUSTAT Flag to receive next GP0 command
	gp0_ResetStatus();

	return true;
}

bool GPU::gp0_CopyVRam2VRam()
{
	//GP0(80h)
	//Copy Rectangle VRAM to VRAM
	uint32_t	param;

	fifo.pop(param);		//Source Coord     (YyyyXxxxh); Xpos counted in halfwords, Ypos counted in rows
	dataSource.x = static_cast<uint16_t>((param & 0x0000ffff)) & 0x3ff;
	dataSource.y = static_cast<uint16_t>((param & 0xffff0000) >> 16) & 0x1ff;

	fifo.pop(param);		//Destination Coord(YyyyXxxxh); Xpos counted in halfwords, Ypos counted in rows
	dataDestination.x = static_cast<uint16_t>((param & 0x0000ffff)) & 0x3ff;
	dataDestination.y = static_cast<uint16_t>((param & 0xffff0000) >> 16) & 0x1ff;

	fifo.pop(param);		//Width+Height     (YsizXsizh); Xsiz counted in halfwords, Ysiz counted in rows
	dataSize.x = ((static_cast<uint16_t>((param & 0x0000ffff)) - 1) & 0x3ff) + 1;
	dataSize.y = ((static_cast<uint16_t>((param & 0xffff0000) >> 16) - 1) & 0x1ff) + 1;

	//Copy from VRAM to VRAM
	//This is done on a single clock cycle to ease the implementation
	uint16_t data;
	for (int y = 0; y < dataSize.y; y++)
		for (int x = 0; x < dataSize.x; x++)
		{
			data = Renderer::ReadVRAM(dataSource.x + x, dataSource.y + y);
			Renderer::WriteVRAM(dataDestination.x + x, dataDestination.y + y, data);
		}
	Renderer::CommitVRAMWrite();
	
	//Reset GPUSTAT Flag to receive next GP0 command
	gp0_ResetStatus();

	return true;
}

bool GPU::gp0_CopyRam2VRam()
{
	//GP0(A0h)
	//Copy Rectangle RAM to VRAM
	uint32_t	param;
	
	fifo.pop(param);		//Destination Coord(YyyyXxxxh); Xpos counted in halfwords, Ypos counted in rows
	dataDestination.x = static_cast<uint16_t>((param & 0x0000ffff)) & 0x3ff;
	dataDestination.y = static_cast<uint16_t>((param & 0xffff0000) >> 16) & 0x1ff;

	fifo.pop(param);		//Width+Height     (YsizXsizh); Xsiz counted in halfwords, Ysiz counted in rows
	dataSize.x = ((static_cast<uint16_t>((param & 0x0000ffff)) - 1) & 0x3ff) + 1;
	dataSize.y = ((static_cast<uint16_t>((param & 0xffff0000) >> 16) - 1) & 0x1ff) + 1;

	//Set Internal Transfer Status and Size
	dataPointer.x = dataDestination.x;
	dataPointer.y = dataDestination.y;
	dataWriteActive = true;

	LOG_F(2, "GPU - Write to VRAM at [x: %d, y: %d]", dataDestination.x, dataDestination.y);
	LOG_F(2, "GPU - Copy Size        [x: %d, y: %d]", dataSize.x, dataSize.y);
	
			 
	//Reset GPUSTAT Flag to receive next GP0 command
	gp0_ResetStatus();

	return true;
}

bool GPU::gp0_CopyVRam2Ram()
{
	//GP0(C0h)
	//Copy Rectangle VRAM to RAM
	uint32_t	param;

	fifo.pop(param);		//Source Coord     (YyyyXxxxh); Xpos counted in halfwords, Ypos counted in rows
	dataSource.x = static_cast<uint16_t>((param & 0x0000ffff)) & 0x3ff;
	dataSource.y = static_cast<uint16_t>((param & 0xffff0000) >> 16) & 0x1ff;

	fifo.pop(param);		//Width+Height     (YsizXsizh); Xsiz counted in halfwords, Ysiz counted in rows	
	dataSize.x = ((static_cast<uint16_t>((param & 0x0000ffff)) - 1) & 0x3ff) + 1;
	dataSize.y = ((static_cast<uint16_t>((param & 0xffff0000) >> 16) - 1) & 0x1ff) + 1;

	//Set Internal Transfer Status and Size
	dataPointer.x = dataSource.x;
	dataPointer.y = dataSource.y;
	dataReadActive = true;

	LOG_F(2, "GPU - Read from VRAM at [x: %d, y: %d]", dataSource.x, dataSource.y);
	LOG_F(2, "GPU - Copy Size         [x: %d, y: %d]", dataSize.x, dataSize.y);
	 
	//Reset GPUSTAT Flag to receive next GP0 command
	gp0_ResetStatus();

	return true;
}

bool GPU::gp0_FillVRam()
{
	//GP0(02h)
	//Fill Rectangle in VRAM
	uint32_t	param;
	uint16_t	color;
	lite::vec2t<uint16_t> position, size;	
	
	color = rgb24torgb15(gp0Command & 0x00ffffff);	//Color + Command(CcBbGgRrh); 24bit RGB value

	fifo.pop(param);		//Top Left Corner(YyyyXxxxh); Xpos counted in halfwords, steps of 10h
	position.x = static_cast<uint16_t>((param & 0x0000ffff)) & 0x3f0;
	position.y = static_cast<uint16_t>((param & 0xffff0000) >> 16) & 0x1ff;

	fifo.pop(param);		//Width + Height(YsizXsizh); Xsiz counted in halfwords, steps of 10h
	size.x = ((static_cast<uint16_t>((param & 0x0000ffff)) & 0x3ff) + 0x0f) & ~(0x0f);
	size.y = static_cast<uint16_t>((param & 0xffff0000) >> 16) & 0x1ff;

	//Fill is not executed if either Xsiz or Ysiz are zero.
	if (size.x == 0 || size.y == 0)
		return true;

	//Fill VRAM with RGB value in color
	//This is done on a single clock cycle to ease the implementation
	for (int y = 0; y < size.y; y++)
		for (int x = 0; x < size.x; x++)
			Renderer::WriteVRAM(position.x + x, position.y + y, color);
	Renderer::CommitVRAMWrite();
		
	//Reset GPUSTAT Flag to receive next GP0 command
	gp0_ResetStatus();

	return true;
}

bool GPU::gp0_ClearTextureCache()
{
	//GP0(01h)
	//Clear Texture Cache
	
	// NOTHING TODO
	
	//Reset GPUSTAT Flag to receive next GP0 command
	gp0_ResetStatus();

	return true;
}

bool GPU::gp0_InterruptRequest()
{
	//GP0(1Fh)
	//Set GPUSTAT.24
	gpuStat |= 1UL << 24;

	//Trigger Interrupt Request
	psx->interrupt->request(static_cast<uint32_t>(interrupt::Cause::gpu));

	//Reset GPUSTAT Flag to receive next GP0 command
	gp0_ResetStatus();

	return true;
}

bool GPU::gp0_DrawMode()
{
	//GP0(E1h)
	//Draw Mode Settings, Texpage. Modify various GPUSTAT bits

	uint32_t data;

	//GPUSTAT.0-3 << GP0DATA.0-3								Texture page X Base (N x 64) in halfwords
	data = gp0DataLatch & 0x0000000f;							//Extract bit value from GP0 Command
	gpuStat = (gpuStat & ~(0x0000000f)) | (data);				//Set GPUSTAT.0-3 to data value
	texturePage.x = (data * 64);

	//GPUSTAT.4 << GP0DATA.4									Texture page Y Base (N x 256) in halfwords
	data = (gp0DataLatch & 0x00000010) >> 4;					//Extract bit value from GP0 Command
	gpuStat = (gpuStat & ~(0x00000001 << 4)) | (data << 4);		//Set GPUSTAT.4 to data value
	texturePage.y = (data * 256);

	//GPUSTAT.5-6 << GP0DATA.5-6								Semi Transparency (0 = B/2+F/2, 1 = B+F, 2 = B-F, 3 = B+F/4)
	data = (gp0DataLatch & 0x000000060) >> 5;					//Extract bit value from GP0 Command
	gpuStat = (gpuStat & ~(0x00000003 << 5)) | (data << 5);		//Set GPUSTAT.5-6 to data value
	semiTransparencyMode = data;

	//GPUSTAT.7-8 << GP0DATA.7-8								Texture page colors (0 = 4bit, 1 = 8bit, 2 = 15bit, 3 = reserved)
	data = (gp0DataLatch & 0x00000180) >> 7;					//Extract bit value from GP0 Command
	gpuStat = (gpuStat & ~(0x00000003 << 7)) | (data << 7);		//Set GPUSTAT.7-8 to data value
	colorMode = data;

	//GPUSTAT.9 << GP0DATA.9									Dither 24bit to 15bit (0 = off, 1 = enabled)
	data = (gp0DataLatch & 0x00000200) >> 9;					//Extract bit value from GP0 Command
	gpuStat = (gpuStat & ~(0x00000001 << 9)) | (data << 9);		//Set GPUSTAT.9 to data value
	ditherEnabled = (bool)data;

	//GPUSTAT.10 << GP0DATA.10									Drawing to display area (0 = prohibited, 1 = allowed)
	data = (gp0DataLatch & 0x00000400) >> 10;					//Extract bit value from GP0 Command
	gpuStat = (gpuStat & ~(0x00000001 << 10)) | (data << 10);	//Set GPUSTAT.10 to data value
	drawingEnabled = (bool)data;

	//GPUSTAT.15 << GP0DATA.11									Texture Disable (0=Normal, 1=Texture Disabled)
	data = (gp0DataLatch & 0x00000800) >> 11;					//Extract bit value from GP0 Command
	gpuStat = (gpuStat & ~(0x00000001 << 15)) | (data << 15);	//Set GPUSTAT.15 to data value

	//Extract Texture Rectangle X-Flip
	data = (gp0DataLatch & 0x00001000) >> 12;					//Extract bit value from GP0 Command
	rectangleTexFlipX = data;									//Bios set up this bit on power up?

	//Extract Texture Rectangle X-Flip
	data = (gp0DataLatch & 0x00002000) >> 13;					//Extract bit value from GP0 Command
	rectangleTexFlipY = data;									//Bios set it equal to GPUSTAT.13?
		
	//Reset GPUSTAT Flag to receive next GP0 command
	gp0_ResetStatus();

	//Update Renderer Status
	Renderer::SetTexturePage(texturePage);
	Renderer::SetTransparencyMode(semiTransparencyMode);
	Renderer::SetTextureColorMode(colorMode);
	Renderer::SetDither(ditherEnabled);
	Renderer::SetDrawingEnabled(drawingEnabled);

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

	//Update Renderer Status
	Renderer::SetTextureMask(textureMask);
	Renderer::SetTextureOffset(textureOffset);

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

	//Update Renderer Status
	Renderer::SetDrawingArea(drawingArea);
	
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

	//Update Renderer Status
	Renderer::SetDrawingArea(drawingArea);

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

	//Update Renderer Status
	Renderer::SetDrawingOffset(drawingOffset);

	//Reset GPUSTAT Flag to receive next GP0 command
	gp0_ResetStatus();

	return true;
}

bool GPU::gp0_SetMaskBit()
{
	//GP0(E6h)
	//Mask Bit Setting

	uint32_t data;
	bool forceMask, checkMask;

	//GPUSTAT.11 << GP0DATA.0									Force Mask while drawing (0 = disable, 1 = enable)
	data = gp0DataLatch & 0x00000001;							//Extract bit value from GP0 Command
	gpuStat = (gpuStat & ~(0x00000001 << 11)) | (data << 11);	//Set GPUSTAT.11 to data value
	forceMask = (bool)data;

	//GPUSTAT.12 << GP0DATA.1									Check mask before draw (0=Draw Always, 1=Draw if Bit15=0)
	data = (gp0DataLatch & 0x00000002) >> 1;					//Extract bit value from GP1 Command
	gpuStat = (gpuStat & ~(0x00000001 << 12)) | (data << 12);	//Set GPUSTAT.12 to data value
	checkMask = (bool)data;

	//Update Renderer Status
	Renderer::SetMaskBit(checkMask, forceMask);

	//Reset GPUSTAT Flag to receive next GP0 command
	gp0_ResetStatus();

	return true;
}

bool GPU::gp0_NoOperation()
{
	//Nothing to do

	//Reset GPUSTAT Flag to receive next GP0 command
	gp0_ResetStatus();

	return true;
}

void GPU::gp0_ResetStatus()
{
	recvCommand = false;
	recvParameters = false;
	gp0RecvPolyLine = false;
	gp0CommandAvailable = false;
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
	gpuStat = (0x14802000 & ~(0x00000001 << 15)) | data;	//Reset GPUSTAT and rewrite old GPUSTAT.15 value

	//Reset status flags to receive next GP1 command
	gp1_ResetStatus();

	return true;
}

bool GPU::gp1_ResetFifo()
{
	//GP1(01h)
	//Flush Command FIFO Buffer 
	fifo.flush();

	//Reset status flags to receive next GP1 command
	gp1_ResetStatus();

	return true;
}

bool GPU::gp1_AckInterrupt()
{
	//GP1(02h)
	//Acnowledge GPU Interrupt, this flag in set by GP0(1Fh)
	//which trigger the INTERRUPT request
	
	//GPUSTAT.24 << 0
	gpuStat &= ~(1UL << 24);

	//Reset status flags to receive next GP1 command
	gp1_ResetStatus();

	return true;
}

bool GPU::gp1_DisplayEnable()
{
	//GP1(03h)
	//Display Enable (0 = on, 1 = off)
		
	uint32_t data;
	
	//GPUSTAT.23 << GP1DATA.0	
	data = gp1Command & 0x00000001;								//Extract bit value from GP1 Command
	gpuStat = (gpuStat & ~(0x00000001 << 23)) | (data << 23);	//Set GPUSTAT.23 to data value
	displayDisabled = (bool)data;

	//Update Renderer Status
	Renderer::SetDisplayDisable(displayDisabled);

	//Reset status flags to receive next GP1 command
	gp1_ResetStatus();
	
	return true;
}

bool GPU::gp1_DmaDirection()
{
	//GP1(04h)
	//DMA Direction / Data Request (0 = off, 1 = FIFO, 2 =  RAM to VRAM, 3 = VRAM to RAM)
	
	uint32_t data;

	//GPUSTAT.29-30 << GP1DATA.0-1
	data = gp1Command & 0x00000003;								//Extract bit value from GP1 Command
	gpuStat = (gpuStat & ~(0x00000003 << 29)) | (data << 29);	//Set GPUSTAT.29-30 to data value

	LOG_F(2, "GPU - DMA Direction Info [Direction: 0x%02x]", data);

	//Reset status flags to receive next GP1 command
	gp1_ResetStatus();

	return true;
}

bool GPU::gp1_StartDisplayArea()
{
	//GP1(05h)
	//Start of Display Area in VRAM (gp1Data.0-9 = X, gp1Data.10-18 = Y)
	 
	displayStart.x = gp1Command & 0x000003ff;
	displayStart.y = (gp1Command & 0x0007fc00) >> 10;

	//Update Renderer Status
	Renderer::SetDisplayStart(displayStart);

	//Reset status flags to receive next GP1 command
	gp1_ResetStatus();

	return true;
}

bool GPU::gp1_HDisplayRange()
{
	//GP1(06h)
	//Horizontal Display Range on Screen (gp1Data.0-11 = X1, gp1Data.12-23 = X2)
		
	displayRange.x1 = gp1Command & 0x00000fff;
	displayRange.x2 = (gp1Command & 0x00fff000) >> 12;

	//Reset status flags to receive next GP1 command
	gp1_ResetStatus();

	return true;
}

bool GPU::gp1_VDisplayRange()
{
	//GP1(07h)
	// Vertical Display Range on Screen (gp1Data.0-9 = Y1, gp1Data.10-19 = Y2)
	 
	displayRange.y1 = gp1Command & 0x000003ff;
	displayRange.y2 = (gp1Command & 0x000ffc00) >> 10;

	//Reset status flags to receive next GP1 command
	gp1_ResetStatus();

	return true;
}

bool GPU::gp1_DisplayMode()
{	
	//GP1(08h)
	//Modify various GPUSTAT bits

	uint32_t data;

	//GPUSTAT.17-18 << GP1DATA.0-1								Horizontal Resolution 1: 0 = 256, 1 = 320, 2 = 512, 3 = 640 					
	data = gp1Command & 0x00000003;								//Extract bit value from GP1 Command
	gpuStat = (gpuStat & ~(0x00000003 << 17)) | (data << 17);	//Set GPUSTAT.17-18 to data value

	//GPUSTAT.19 << GP1DATA.2									Vertical Resolution: 0 = 240, 1 = 480
	data = (gp1Command & 0x00000004) >> 2;						//Extract bit value from GP1 Command
	gpuStat = (gpuStat & ~(0x00000001 << 19)) | (data << 19);	//Set GPUSTAT.19 to data value

	//GPUSTAT.20 << GP1DATA.3									Video Mode: 0 = NTSC, 1 = PAL
	data = (gp1Command & 0x00000008) >> 3;						//Extract bit value from GP1 Command
	gpuStat = (gpuStat & ~(0x00000001 << 20)) | (data << 20);	//Set GPUSTAT.20 to data value
	displayMode = static_cast<VideoMode>(data);
	switch(displayMode)
	{
		NTSC:
			tickCountPerScanline = NTSC_GPU_CLOCK_PER_SCANLINE;
			tickCountPerHBlank = NTSC_GPU_CLOCK_PER_HBLANK;
			scanlinePerFrame = NTSC_SCANLINES_PER_FRAME;
			scanlinePerVBlank = NTSC_SCANLINES_PER_VBLANK;
			break;
		PAL:
			tickCountPerHBlank = PAL_GPU_CLOCK_PER_HBLANK;
			tickCountPerScanline = PAL_GPU_CLOCK_PER_SCANLINE;
			scanlinePerFrame = PAL_SCANLINES_PER_FRAME;
			scanlinePerVBlank = PAL_SCANLINES_PER_VBLANK;
			break;
	}

	//GPUSTAT.21 << GP1DATA.4									Display Area Color Depth: 0 = 15bit, 1 = 24bit
	data = (gp1Command & 0x00000010) >> 4;						//Extract bit value from GP1 Command
	gpuStat = (gpuStat & ~(0x00000001 << 21)) | (data << 21);	//Set GPUSTAT.21 to data value
	displayColorMode = static_cast<uint8_t>(data);

	//GPUSTAT.22 << GP1DATA.5									Vertical Interlace: 0 = off, 1 = on
	data = (gp1Command & 0x00000020) >> 5;						//Extract bit value from GP1 Command
	gpuStat = (gpuStat & ~(0x00000001 << 22)) | (data << 22);	//Set GPUSTAT.22 to data value
	verticalInterlace = static_cast<bool>(data);

	//GPUSTAT.16 << GP1DATA.6									Horizontal Resolution 2: 0 = 256/320/512/640, 1 = 368
	data = (gp1Command & 0x00000040) >> 6;						//Extract bit value from GP1 Command
	gpuStat = (gpuStat & ~(0x00000001 << 16)) | (data << 16);	//Set GPUSTAT.16 to data value

	//GPUSTAT.14 << GP1DATA.7									//Reverse Flag: 0 = normal, 1 = distorted
	data = (gp1Command & 0x00000080) >> 7;						//Extract bit value from GP1 Command
	gpuStat = (gpuStat & ~(0x00000001 << 14)) | (data << 14);	//Set GPUSTAT.16 to data value

	//Extract Current Video Resolution in readable form from gpuStat new value
	displayResolution = decodeResolution(gpuStat);

	//Set DotClock Divider according to new Horizontal Resolution configuration
	dotClockRatio = decodeDotClockRatio(gpuStat);
	tickCountPerDots = tickCountPerScanline / dotClockRatio;

	//Update Renderer Resolution
	Renderer::SetDisplayResolution(displayResolution);
	Renderer::SetDisplayInterlaceMode(verticalInterlace);
	Renderer::SetDisplayColorMode(displayColorMode);

	//Reset status flags to receive next GP1 command
	gp1_ResetStatus();

	return true;
}

bool GPU::gp1_NewTextureDisable()
{
	//GP1(09h)
	//New Texture Disable (0 = normal, 1 = disabled)
	//Avoid updating Global Texture Configuration using
	//information received from Primitive Rendering Commands

	uint32_t data;

	//GPUSTAT.15 << GP1DATA.0
	data = (gp1Command & 0x00000001);							//Extract bit value from GP1 Command
	textureDisabled = (bool)data;

	//TODO: if New Texture are disabled, ignore texture information from Primitive Rendering Commands

	//Reset GPUSTAT Flag to receive next GP1 command
	gp1_ResetStatus();

	return true;
}

bool GPU::gp1_Unknown()
{	
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

	data = gp1Command & 0x00000007;
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