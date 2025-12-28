#pragma once

class Psx;

class Device
{
public:
    virtual ~Device() = default;
    virtual bool runTicks(uint32_t cycles) = 0;

public:
    // Link to PSX instance
    virtual void link(Psx* instance) = 0;

protected:
    float   deviceClockFrequency;   //Device Clock Frequency
    float   cpuClockFrequency;      //CPU Clock Frequency
    float   clockRatio;             //Device to cpu Clock Ratio (CPU Clock is the Master Clock)
    float   ticks;                  //Device Number of accumulated clock Ticks
};
