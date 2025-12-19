#pragma once

#include <cstdint>
#include <cstring>
#include <cstdio>

#include "litelib.h"

namespace cop0
{
	union Operation
	{
		uint32_t		word;
		
		lite::bitfield<0, 6>  funct;			//Cop0 functions
		lite::bitfield<11, 5> rd;				//rd index
		lite::bitfield<16, 5> rt;				//rt index
		lite::bitfield<21, 5> operation;		//Cop0 Sub Operation
	};

   	union CauseRegister
	{
		uint32_t		word;

		lite::bitfield<2, 5>	excode;	//exception code
		lite::bitfield<8, 2>	ipsw;	//Software Interrupt Pending
		lite::bitfield<10, 6>	iphw;	//Hardware Interrupt Pending
		lite::bitfield<28, 2>	ce;		//Coprocessor number exception
		lite::bitfield<31, 1>	bd;		//Branch Delay Exception
	};

	union StatusRegister
	{
		uint32_t		word;

		lite::bitfield<0, 1>	iec;	//Current Interrupt enable
		lite::bitfield<1, 1>	kuc;	//Current Kernel/User mode
		lite::bitfield<2, 1>	iep;	//Previous Interrupt enable
		lite::bitfield<3, 1>	kup;	//Previous Kernel/User mode
		lite::bitfield<4, 1>	ieo;	//Old Interrupt enable
		lite::bitfield<5, 1>	kuo;	//Old Kernel/User mode
		lite::bitfield<8, 2>	imsw;	//Sotfware Interrupt Mask
		lite::bitfield<10, 6>	imhw;	//Hardware Interrupt Mask
		lite::bitfield<16, 1>	isc;	//Isolate Cache
		lite::bitfield<17, 1>	swc;	//Swappe Cache mode
		lite::bitfield<18, 1>	pz;		//When set parity bit are written az 0's
		lite::bitfield<19, 1>	cm;		//Set if last access to D-Cache contained actual data
		lite::bitfield<20, 1>	pe;		//Cache parity check
		lite::bitfield<21, 1>	ts;		//TLB Shutdown
		lite::bitfield<22, 1>	bev;	//Boot Exception Vector (0 = RAM, 1 = ROM)
		lite::bitfield<25, 1>	re;		//Reverse Endianess (0 = normal endianess, 1 = 0 reversed endianess)
		lite::bitfield<28, 1>	cu0;	//COP0 Enable
		lite::bitfield<29, 1>   cu1;	//COP1 Enable
		lite::bitfield<30, 1>	cu2;	//COP2 Enable
		lite::bitfield<31, 1>	cu3;	//COP3 Enable

		lite::bitfield<0, 6>	stk;	//Interrupt Enable / Kernel Mode Stack
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