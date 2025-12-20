#include <loguru.hpp>
#include "cpu_full_pipe.h"
#include "psx.h"

CpuFull::CpuFull()
{
}

CpuFull::~CpuFull() = default;

// Lifecycle
bool CpuFull::reset()
{
    return false;
}

bool CpuFull::execute()
{
    return false;
}

// Cache & Memory Access
uint32_t CpuFull::rdInst(uint32_t vAddr, uint8_t bytes)
{
    return 0;
}

uint32_t CpuFull::rdMem(uint32_t vAddr, uint8_t bytes)
{
    return 0;
}

bool CpuFull::wrMem(uint32_t vAddr, uint32_t& data, uint8_t bytes, bool checkalign)
{
    return false;
}

uint32_t CpuFull::rdDataCache(uint32_t vAddr, uint8_t bytes)
{
    return 0;
}

bool CpuFull::wrDataCache(uint32_t vAddr, uint32_t& data, uint8_t bytes)
{
    return false;
}

uint32_t CpuFull::rdInstrCache(uint32_t vAddr, uint8_t bytes)
{
    return 0;
}

bool CpuFull::wrInstrCache(uint32_t vAddr, uint32_t& data, uint8_t bytes)
{
    return false;
}

// Pipeline phase functions
bool CpuFull::exception(uint32_t cause)
{
    return false;
}

bool CpuFull::interrupt(uint8_t status)
{
    return false;
}

// Basic register accessors
uint32_t CpuFull::get_pc() const
{
    return pc;
}

void CpuFull::set_pc(uint32_t value)
{
    pc = value;
}

uint32_t CpuFull::get_hi() const
{
    return hi;
}

void CpuFull::set_hi(uint32_t value)
{
    hi = value;
}

uint32_t CpuFull::get_lo() const
{
    return lo;
}

void CpuFull::set_lo(uint32_t value)
{
    lo = value;
}

uint32_t CpuFull::get_gpr(uint8_t regNumber) const
{
    return gpr[regNumber];
}

void CpuFull::set_gpr(uint8_t regNumber, uint32_t value)
{
    gpr[regNumber] = value;
}
