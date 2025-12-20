#pragma once

#include <cstdio>
#include <vector>

#include "fifo.h"

class Psx;

class Controller
{
public:
    Controller();
    ~Controller();

    bool execute();
    bool reset();

    bool writeAddr(uint32_t addr, uint32_t& data, uint8_t bytes);
	uint32_t readAddr(uint32_t addr, uint8_t bytes);

    //Connect to PSX Instance
	void link(Psx* instance) { psx = instance; }

private:
    //Link to PSP Object
	Psx* psx;

    //Joy Status Register
    uint32_t    joyStat;

    //Joy Mode Register
    uint32_t    joyMode;

    //Joy Control Register
    uint16_t    joyCntr;

    //Joy RX FIFO
	lite::fifo<uint8_t, 8>	rxfifo;   

    //Joy TX Data
    uint32_t    txData;
};