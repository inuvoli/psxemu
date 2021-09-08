#pragma once

#include <cstdint>

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