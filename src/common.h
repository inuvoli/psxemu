#pragma once

#include <cstdint>

template <typename T>
struct vec2t
{
	T	x;
	T	y;

	vec2t& operator= (const vec2t& t)
	{
		x = t.x;
		y = t.y;
		return *this;
	}
};

template <typename T>
struct vec3t
{
	T	x;
	T	y;
	T	z;

	vec3t& operator= (const vec3t& t)
	{
		x = t.x;
		y = t.y;
		z = t.z;
		return *this;
	}
};

template <typename T>
struct vec4t
{
	T	x1;
	T	y1;
	T	x2;
	T	y2;

	vec4t& operator= (const vec4t& t)
	{
		x1 = t.x1;
		y1 = t.y1;
		x2 = t.x2;
		y2 = t.y2;
		return *this;
	}
};

/// <summary>
/// Check if an Address is within the range [lowValue, highValue)
/// </summary>
/// <param name="inputValue">: Address to be checked</param>
/// <param name="lowValue">: Lower bound of the range</param>
/// <param name="highValue">: Higher bound of the range</param>
/// <returns>True if the address is in range, False id the address is not in range</returns>
inline bool isInRange(uint32_t inputValue, uint32_t lowValue, uint32_t highValue)
{
	//Check if inputValue is included in [lowValue, HighValue)
	return  ((inputValue - lowValue) < (highValue - lowValue));
}

//inline bool isInRange(uint32_t inputValue, uint32_t startValue, uint32_t lenghtByte)
//{
//	//Check if inputValue is included in [startValue, startValue + lenghtByte)
//	return  ((inputValue - startValue) < lenghtByte);
//}

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

//GPU Parameters Decode Helper Functions
inline void decodeColor(const uint32_t param, vec3t<uint8_t> &color)
{
	color.x = (param >> 16) & 0x000000ff;	//RED
	color.y = (param >> 8) & 0x000000ff;	//GREEN
	color.z = param & 0x000000ff;			//BLUE
}

inline void decodePosition(const uint32_t param, vec2t<int16_t> &position)
{
	position.x = param & 0x0000ffff;
	position.y = (param >> 16) & 0x0000ffff;
}

inline uint16_t decodeTextureUV(const uint32_t param, vec2t<uint8_t> &texCoords)
{
	texCoords.x = (param >> 8) & 0x000000ff; 	//U Coordinate
	texCoords.y = param & 0x000000ff;			//V Coordinates

	return (uint16_t)(param >> 16);
}

inline void decodeClut(const uint16_t param, vec2t<uint16_t> &clutCoords)
{
	clutCoords.x = param & 0x003f;			//X Coordinates for CLUT in halfwords (16 bits) on the framebuffer
	clutCoords.y = (param >> 6) & 0x01ff;	//y Coordinates for CLUT in Lines on the framebuffer
}
