#pragma once
#include <cstdint>
#include <cstdio>

class Psx;

class Tty
{
public:
    Tty();
    ~Tty();

    bool writeAddr(uint32_t addr, uint32_t& data, uint8_t bytes);
	uint32_t readAddr(uint32_t addr, uint8_t bytes);
    
    //Connect to PSX Instance
	void link(Psx* instance) { psx = instance; }

private:
    //Link to PSP Object
	Psx* psx;
};