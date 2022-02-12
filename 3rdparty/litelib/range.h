#pragma once

#include<cstdint>

//-------------------------------------------------------------------------------------
// Range Class
//-------------------------------------------------------------------------------------
namespace lite
{
	class range
	{
		public:
		range(uint32_t _startAddr, uint32_t _rangeLenght) : startAddr(_startAddr), rangeLenght(_rangeLenght) {};
		
		inline bool contains(uint32_t addr)
		{
			return  ((addr - startAddr) < rangeLenght);
		};

	private:
		uint32_t startAddr;
		uint32_t rangeLenght;
	};
};
