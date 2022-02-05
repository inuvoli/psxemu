#pragma once

#include <cstdint>
#include <memory>

#include "psx.h"

#include "vectors.h"
#include "mipsdisassembler.h"

constexpr auto ModulesNumber = 15;
enum class StepMode { Halt = 0, Manual = 1, Instruction = 2, Frame = 3 };
enum class DebugModule { Bios = 0, Ram = 1, Cpu = 2, Code = 3, Dma = 4, Timers = 5, Gpu = 6, Spu = 7, Cdrom = 8 };





class Psx;

class Debugger
{
public:
    Debugger(const std::shared_ptr<Psx>& instance);
    ~Debugger();

    bool isBreakpoint (uint32_t addr) { return (addr == (breakPoint - 0x4)); };
    bool setBreakpoint(uint32_t addr) { breakPoint = addr; };
    uint32_t getBreakPoint() { return breakPoint; };

    void setStepMode(StepMode mode) { stepMode = mode; };
    StepMode getStepMode() { return stepMode; };

    void toggleDebugModuleStatus(DebugModule module);
    bool getDebugModuleStatus(DebugModule module);

    bool update();

private:
	//Link to Bus Object
    std::shared_ptr<Psx> psx;

private:
	//Debug Status
    bool        debugModuleStatus[ModulesNumber] = {};
    
	uint32_t	breakPoint;
    StepMode    stepMode;
    //Component Status


};