#pragma once
#include <cstdint>
#include <cstdio>
#include <vector>
#include <string>

#include <iostream>
#include <string>
#include <fstream>

class Psx;

class Tty
{
public:
    Tty();
    ~Tty();

    std::string buffer;
    std::vector<std::string> bufferA;
    std::vector<std::string> bufferB;

    bool reset();
    
    bool writeTTYChar(char c);
    bool writeAddr(uint32_t addr, uint32_t& data, uint8_t bytes);
	uint32_t readAddr(uint32_t addr, uint8_t bytes);
    
    //Connect to PSX Instance
	void link(Psx* instance) { psx = instance; }

private:
    //Link to PSP Object
	Psx* psx;

    std::ofstream ofs;
};