#pragma once

//-------------------------------------------------------------------------------------
// Generic Utility Function for PSX
//-------------------------------------------------------------------------------------

class utility
{
private:
    utility() {}

public:
    static bool Virtual2PhisicalAddr(uint32_t vAddr, uint32_t& phAddr)
    {
        uint32_t maskIndex;

        //Mask Region MSBs and Check if Region is Cached
        maskIndex = vAddr >> 29;
        phAddr = vAddr & regionMask[maskIndex];
        return cacheMask[maskIndex];
    };

private:
    //Memory Conversion Helper
	static constexpr uint32_t	regionMask[8] = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x7fffffff, 0x1fffffff, 0xffffffff, 0xffffffff };
	static constexpr bool		cacheMask[8] = { true, true, true, true, true, false, true, true };
};
