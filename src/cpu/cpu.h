// cpu interface
#pragma once

#include <cstdint>
#include <memory>

#include "cpu_registers.h"
#include "cop0.h"
#include "cop2.h"
#include "debugger.h"

using KernelCallCallback = std::function<void(KernelCallEvent &e)>;

class Psx;

class CPU
{
public:
	virtual ~CPU() = default;

	// Lifecycle
	virtual bool reset() = 0;
	virtual bool execute() = 0;

	// Cache & Memory Access
	virtual uint32_t rdInst(uint32_t vAddr, uint8_t bytes = 4) = 0;
	virtual uint32_t rdMem(uint32_t vAddr, uint8_t bytes = 4) = 0;
	virtual bool     wrMem(uint32_t vAddr, uint32_t& data, uint8_t bytes = 4, bool checkalign = true) = 0;
	virtual uint32_t rdDataCache(uint32_t vAddr, uint8_t bytes) = 0;
	virtual bool     wrDataCache(uint32_t vAddr, uint32_t& data, uint8_t bytes) = 0;
	virtual uint32_t rdInstrCache(uint32_t vAddr, uint8_t bytes) = 0;
	virtual bool     wrInstrCache(uint32_t vAddr, uint32_t& data, uint8_t bytes) = 0;

	// Pipeline phase functions
	virtual bool exception(uint32_t cause) = 0;
	virtual bool interrupt(uint8_t number, bool status) = 0;

	// Link to PSX instance
	virtual void link(Psx* instance) = 0;

	// Basic register accessors
	virtual uint32_t get_pc() const = 0;
	virtual void     set_pc(uint32_t value) = 0;
	virtual uint32_t get_hi() const = 0;
	virtual void     set_hi(uint32_t value) = 0;
	virtual uint32_t get_lo() const = 0;
	virtual void     set_lo(uint32_t value) = 0;
	virtual uint32_t get_gpr(uint8_t regNumber) const = 0;
	virtual void	 set_gpr(uint8_t regNumber, uint32_t value) = 0;

	// Debug Call Stack Callback
	virtual void     setKernelCallCallback(KernelCallCallback cb) = 0;

public:
	//CPU Internal Registers
	uint32_t	pc;					//Program Counter
	uint32_t	hi;					//HI Register, used for Mult and Div
	uint32_t	lo;					//LO Register, used for Mult and Div
	uint32_t	gpr[32];			//CPU General Purpose Registers
	uint32_t	cacheReg;			//Cache Control Register (mapped to 0xfffe0130)

    //Internal Status Flags
    bool        isInDelaySlot;     	//Indicates if the current instruction is in a delay slot

    //Additional Info
    uint32_t    branchAddress;      	//branch target address for jump/branch instructions
	uint32_t	branchFunctionAddress;	//jump/branch function address, used for correct EPC setting on exceptions during delay slots

    //Coprocessors
    std::shared_ptr<Cop0>	cop0;	//Coprocessor 0
    std::shared_ptr<Cop2>	cop2;	//Coprocessor 2 (GTE)

protected:
    //Link to Bus Object
    Psx* psx;
};
