#pragma once

#include <cstdint>
#include <cstring>
#include <cstdio>

class CPU;

class Cop2
{
public:
    Cop2(CPU* instance);
    ~Cop2();

    //Cop2 Interface
    bool reset();
    bool execute (uint32_t cofun);

public:
    //Cop0 Registers
    uint32_t    reg[64];

private:
	CPU* cpu;
};