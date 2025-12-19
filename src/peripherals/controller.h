#pragma once

#include <cstdio>
#include <vector>

#include "device.h"
#include "fifo.h"

class Controller : public Idevice
{
public:
    Controller();
    ~Controller();

    bool execute() override;
    bool reset() override;

    bool writeAddr(uint32_t addr, uint32_t& data, uint8_t bytes) override;
	uint32_t readAddr(uint32_t addr, uint8_t bytes) override;

private:
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