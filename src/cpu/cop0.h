#pragma once

#include <cstdint>
#include <cstring>
#include <cstdio>

#include "bitfield.h"

namespace cop0
{
	union Operation
	{
		uint32_t		word;
		
		BitField<0, 6>  funct;			//Cop0 functions
		BitField<11, 5> rd;				//rd index
		BitField<16, 5> rt;				//rt index
		BitField<21, 5> operation;		//Cop0 Sub Operation
	};

   	union CauseRegister
	{
		uint32_t		word;

		BitField<2, 5>	excode;	//exception code
		BitField<8, 2>	ipsw;	//Software Interrupt Pending
		BitField<10, 6>	iphw;	//Hardware Interrupt Pending
		BitField<28, 2>	ce;		//Coprocessor number exception
		BitField<31, 1>	bd;		//Branch Delay Exception
	};

	union StatusRegister
	{
		uint32_t		word;

		BitField<0, 1>	iec;	//Current Interrupt enable
		BitField<1, 1>	kuc;	//Current Kernel/User mode
		BitField<2, 1>	iep;	//Previous Interrupt enable
		BitField<3, 1>	kup;	//Previous Kernel/User mode
		BitField<4, 1>	ieo;	//Old Interrupt enable
		BitField<5, 1>	kuo;	//Old Kernel/User mode
		BitField<8, 2>	imsw;	//Sotfware Interrupt Mask
		BitField<10, 6>	imhw;	//Hardware Interrupt Mask
		BitField<16, 1>	isc;	//Isolate Cache
		BitField<17, 1>	swc;	//Swappe Cache mode
		BitField<18, 1>	pz;		//When set parity bit are written az 0's
		BitField<19, 1>	cm;		//Set if last access to D-Cache contained actual data
		BitField<20, 1>	pe;		//Cache parity check
		BitField<21, 1>	ts;		//TLB Shutdown
		BitField<22, 1>	bev;	//Boot Exception Vector (0 = RAM, 1 = ROM)
		BitField<25, 1>	re;		//Reverse Endianess (0 = normal endianess, 1 = 0 reversed endianess)
		BitField<28, 1>	cu0;	//COP0 Enable
		BitField<29, 1> cu1;	//COP1 Enable
		BitField<30, 1>	cu2;	//COP2 Enable
		BitField<31, 1>	cu3;	//COP3 Enable

		BitField<0, 6>	stk;	//Interrupt Enable / Kernel Mode Stack
	};

}

class CPU;

class Cop0
{
public:
    Cop0(CPU* instance);
    ~Cop0();

    //Cop0 Interface
    bool reset();
    bool execute (uint32_t cofun);

public:
    //Cop0 Registers
    uint32_t    reg[32];

private:
	CPU* cpu;
};