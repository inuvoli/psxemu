#pragma once

#include <cstdint>
#include <cstdio>
#include "GL/glew.h"
#include <glm/glm.hpp>

#include "litelib.h"

//GPU Vertex Structure
struct GpuVertex
{
	uint16_t    x;      // Screen X
	uint16_t    y;      // Screen Y
	uint8_t     r;      // Red
	uint8_t     g;      // Green
	uint8_t     b;      // Blue
	uint8_t     u;      // Texture U
	uint8_t     v;      // Texture V
};

//-------------------------------------------------------------
//GPU Parameters Decode Helper Functions
//-------------------------------------------------------------
inline uint16_t rgb24torgb15(uint32_t data, uint8_t alpha = 0x00)
{
	uint16_t rgb15Out = 0;
	uint8_t tmp;
	
	for (int i = 0; i < 3; i++)
	{
		tmp = (data >> (8 * i + 3)) & 0x1f;
		rgb15Out |= tmp << (5 * i);
	}

	rgb15Out |= (alpha & 0x01) << 15;

	return rgb15Out;
}

inline void decodeColor(const uint32_t param, GpuVertex &color)
{
	color.r = (uint8_t)(param & 0x000000ff);				//RED
	color.g = (uint8_t)((param >> 8) & 0x000000ff);			//GREEN
	color.b = (uint8_t)((param >> 16) & 0x000000ff);		//BLUE
}

inline void decodePosition(const uint32_t param, GpuVertex &position)
{
	position.x = (uint16_t)(param & 0x0000ffff);
	position.y = (uint16_t)((param >> 16) & 0x0000ffff);
}

inline uint16_t decodeTexture(const uint32_t param, GpuVertex &texCoords)
{
	texCoords.u = (uint8_t)(param & 0x000000ff);			//U Coordinates
	texCoords.v = (uint8_t)((param >> 8) & 0x000000ff); 	//V Coordinate

	return (uint16_t)(param >> 16);
}

inline void decodeClut(const uint16_t param, lite::vec2t<uint16_t> &clutCoords)
{
	clutCoords.x = (uint16_t)((param & 0x003f) * 16);		//X Coordinates for CLUT in halfwords (16 bits) on the framebuffer
	clutCoords.y = (uint16_t)((param >> 6) & 0x01ff);		//y Coordinates for CLUT in Lines on the framebuffer
}

inline void decodeTexPage(const uint16_t param, lite::vec2t<uint16_t> &texPageCoords, uint8_t &texColorMode, uint8_t semiTransMode)
{
	texPageCoords.x = (uint16_t)((param & 0xf) * 64);			
	texPageCoords.y = (uint16_t)(((param >> 4) & 0x1) * 256);
	semiTransMode = (uint8_t)((param >> 5) & 0x3);  
	texColorMode = (uint8_t)((param >> 7) & 0x3);
	//texDisabled = (bool)((param >> 10) & 0x1);
}

inline lite::vec2t<uint16_t> decodeResolution(uint32_t gpuStat)
{
	lite::vec2t<uint16_t> resolution;
	uint8_t hRes1, hRes2, vRes1, vMode, iMode;

	hRes1 = (gpuStat >> 17) & 0x3;
	hRes2 = (gpuStat >> 16) & 0x1;
	vRes1 = (gpuStat >> 19) & 0x1;
	vMode = (gpuStat >> 20) & 0x1;  //Video Mode (PAL/NTSC)
	iMode = (gpuStat >> 22) & 0x1;	//Interlace Mode (On/Off)
	resolution = { 0,0 };


	if (hRes2 == 1)
	{
		resolution.x = 368;
	}
	else if (hRes2 == 0)
	{
		switch (hRes1)
		{
		case 0:
			resolution.x = 256;
			break;
		case 1:
			resolution.x = 320;
			break;
		case 2:
			resolution.x = 512;
			break;
		case 3:
			resolution.x = 640;
			break;
		}
	}

	if (vMode == 0) //NTSC
	{
		resolution.y = (vRes1) ? 480 : 240; 
	}
	else if(vMode == 1) //PAL
	{
		resolution.y = (vRes1) ? 512 : 256; 
	}

	return resolution;
}

inline uint8_t decodeDotClockRatio(uint32_t gpuStat)
{
	uint8_t dotClockRatio = 0;
	lite::vec2t<uint16_t> res;

	res = decodeResolution(gpuStat);
	switch(res.x)
	{
	case 256:
		dotClockRatio = 10;	//H Res = 256
		break;
	case 320:
		dotClockRatio = 8;	//H Res = 320
		break;
	case 368:
		dotClockRatio = 7; //H Res = 368
		break;
	case 512:
		dotClockRatio = 5;	//H Res = 512
		break;
	case 640:
		dotClockRatio = 4;	//H Res = 640
		break;
	default:
		dotClockRatio = 0;
	}

	return dotClockRatio;
}