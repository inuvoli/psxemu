#pragma once

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
#include <map>
#include <memory>

#include "litelib.h" 
#include "cpu.h"

class Psx;

class CpuFull : public CPU
{
public:
    CpuFull();
    ~CpuFull() override;

    // Lifecycle
    bool reset() override;
    bool execute() override;

    // Cache & Memory Access
    uint32_t rdInst(uint32_t vAddr, uint8_t bytes = 4) override;
    uint32_t rdMem(uint32_t vAddr, uint8_t bytes = 4) override;
    bool     wrMem(uint32_t vAddr, uint32_t& data, uint8_t bytes = 4, bool checkalign = true) override;
    uint32_t rdDataCache(uint32_t vAddr, uint8_t bytes) override;
    bool     wrDataCache(uint32_t vAddr, uint32_t& data, uint8_t bytes) override;
    uint32_t rdInstrCache(uint32_t vAddr, uint8_t bytes) override;
    bool     wrInstrCache(uint32_t vAddr, uint32_t& data, uint8_t bytes) override;

    // Pipeline phase functions
    bool exception(uint32_t cause) override;
    bool interrupt(uint8_t number, bool status) override;

    // Link to PSX instance
    void link(Psx* instance) { psx = instance; }

    // Basic register accessors
    uint32_t get_pc() const override;
    void     set_pc(uint32_t value) override;
    uint32_t get_hi() const override;
    void     set_hi(uint32_t value) override;
    uint32_t get_lo() const override;
    void     set_lo(uint32_t value) override;
    uint32_t get_gpr(uint8_t regNumber) const override;
    void     set_gpr(uint8_t regNumber, uint32_t value) override;

private:
	//Link to Bus Object
	Psx* psx;
};
