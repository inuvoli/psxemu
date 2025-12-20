#pragma once

#include <cstdint>
#include <cstdio>
#include <glm/glm.hpp>
#include "litelib.h"

//-------------------------------------------------------------
//
//GPU Parameters Decode Helper Functions
//
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

inline void decodeColor(const uint32_t param, glm::vec3 &color)
{
	color.r = (GLfloat)(param & 0x000000ff);			//RED
	color.g = (GLfloat)((param >> 8) & 0x000000ff);		//GREEN
	color.b = (GLfloat)((param >> 16) & 0x000000ff);	//BLUE
}

inline void decodePosition(const uint32_t param, glm::vec2 &position)
{
	int16_t _x, _y;
	_x = (int16_t)(param & 0x0000ffff);
	_y = (int16_t)((param >> 16) & 0x0000ffff);
	position.x = (GLfloat)_x;
	position.y = (GLfloat)_y;
}

inline uint16_t decodeTexture(const uint32_t param, glm::vec2 &texCoords)
{
	texCoords.x = (GLfloat)(param & 0x000000ff);			//U Coordinates
	texCoords.y = (GLfloat)((param >> 8) & 0x000000ff); 	//V Coordinate

	return (uint16_t)(param >> 16);
}

inline void decodeClut(const uint16_t param, glm::vec2 &clutCoords)
{
	clutCoords.x = (GLfloat)((param & 0x003f) * 16);			//X Coordinates for CLUT in halfwords (16 bits) on the framebuffer
	clutCoords.y = (GLfloat)((param >> 6) & 0x01ff);			//y Coordinates for CLUT in Lines on the framebuffer
}

inline void decodeTexPage(const uint16_t param, glm::vec2 &texPageCoords, glm::float32 &texColorDepth, glm::float32 transMode)
{
	texPageCoords.x = (GLfloat)((param & 0xf) * 64);			
	texPageCoords.y = (GLfloat)(((param >> 4) & 0x1) * 256);
	transMode = (GLfloat)((param >> 5) & 0x3);
	texColorDepth = (GLfloat)((param >> 7) & 0x3);
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

inline uint8_t decodeClockRatio(uint16_t hRes)
{
	uint8_t dotClockRatio = 0;

	switch (hRes)
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
