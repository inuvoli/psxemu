#pragma once
//Generic Device Object Class-Interface
//All Object connected to Bus implement this interface

#include <cstdint>

//Forward declaration
class Psx;

class Ibus
{
public:
    //Bus interface functions
    virtual bool execute() = 0;
    virtual bool reset() = 0;
};

class Idevice
{
public:
    //Device interface functions
    virtual bool execute() = 0;
    virtual bool reset() = 0;
    virtual bool writeAddr(uint32_t addr, uint32_t& data, uint8_t bytes) = 0;
	virtual uint32_t readAddr(uint32_t addr, uint8_t bytes) = 0;
    

    //Connect to PSX Instance
	void link(Psx* instance) { psx = instance; };

protected:
    Psx * psx;
};