#pragma once

#include <cstdio>

#include "device.h"

class Controller : public Idevice
{
public:
    Controller();
    ~Controller();

    bool clock() override;
    bool reset() override;
    bool writeAddr(uint32_t addr, uint32_t& data, uint8_t bytes) override;
	uint32_t readAddr(uint32_t addr, uint8_t bytes) override;
};