#pragma once

#include <cstdio>
#include <vector>

#include "litelib.h"

namespace controller
{
    union StatusRegister
    {
        uint32_t word;

        lite::bitfield<0, 1>   txready1;		//TX Ready Flag 1   (1=Ready/Started)
		lite::bitfield<1, 1>   rxfifonoempty;	//RX FIFO Not Empty (0=Empty, 1=Not Empty)
        lite::bitfield<2, 1>   txready2;		//TX Ready Flag 2   (1=Ready/Finished)
        lite::bitfield<3, 1>   rxparityerror;	//RX Parity Error   (0=No, 1=Error; Wrong Parity, when enabled)  (sticky)
        lite::bitfield<7, 1>   ackinputlevel;	///ACK Input Level  (0=High, 1=Low)
        lite::bitfield<9, 1>   interruptreq;	//Interrupt Request (0=None, 1=IRQ7) (See JOY_CTRL.Bit4,10-12)   (sticky)
        lite::bitfield<11, 21> baudratetimer;   //Baudrate Timer    (21bit timer, decrementing at 33MHz)
    };

    union ModeRegister
    {
        uint16_t word;

        lite::bitfield<0, 2>   baudmult;       //Baudrate Reload Factor (1=MUL1, 2=MUL16, 3=MUL64) (or 0=MUL1, too)
        lite::bitfield<2, 2>   charlength;     //Character Length       (0=5bits, 1=6bits, 2=7bits, 3=8bits)
        lite::bitfield<4, 1>   parityenable;   //Parity Enable          (0=No, 1=Enable)
        lite::bitfield<5, 1>   paritytype;     //Parity Type            (0=Even, 1=Odd) (seems to be vice-versa...?)
        lite::bitfield<8, 1>   clkpolarity;    //CLK Output Polarity    (0=Normal:High=Idle, 1=Inverse:Low=Idle)        
    };

    union ControlRegister
    {
        uint16_t word;
            
        lite::bitfield<0, 1>   txen;             //TX Enable (TXEN)  (0=Disable, 1=Enable)
        lite::bitfield<1, 1>   joyoutput;        //JOYn Output      (0=High, 1=Low/Select) (/JOYn as defined in Bit13)
        lite::bitfield<2, 1>   rxen;             //RX Enable (RXEN)  (0=Normal, when /JOYn=Low, 1=Force Enable Once)
        lite::bitfield<4, 1>   ack;              //Acknowledge       (0=No change, 1=Reset JOY_STAT.Bits 3,9)          (W)
        lite::bitfield<5, 1>   unknown5;         //
        lite::bitfield<6, 1>   reset;            //Reset             (0=No change, 1=Reset most JOY_registers to zero) (W)
        lite::bitfield<8, 2>   rxintmode;        //RX Interrupt Mode    (0..3 = IRQ when RX FIFO contains 1,2,4,8 bytes)
        lite::bitfield<10, 1>  txintenable;      //TX Interrupt Enable  (0=Disable, 1=Enable) ;when JOY_STAT.0-or-2 ;Ready
        lite::bitfield<11, 1>  rxintenable;      //RX Interrupt Enable  (0=Disable, 1=Enable) ;when N bytes in RX FIFO
        lite::bitfield<12, 1>  ackintenable;     //ACK Interrupt Enable (0=Disable, 1=Enable) ;when JOY_STAT.7  ;/ACK=LOW
        lite::bitfield<13, 1>  slot;             //Desired Slot Number  (0=/JOY1, 1=/JOY2) (set to LOW when Bit1=1)        
    };
}

class Psx;

class Controller
{
public:
    Controller();
    ~Controller();

    bool execute();
    bool reset();

    bool writeAddr(uint32_t addr, uint32_t& data, uint8_t bytes);
	uint32_t readAddr(uint32_t addr, uint8_t bytes);

    //Connect to PSX Instance
	void link(Psx* instance) { psx = instance; }

private:
    //Link to PSP Object
	Psx* psx;

    //Internal Registers
    controller::StatusRegister  statRegister;
    controller::ModeRegister    modeRegister;
    controller::ControlRegister ctrlRegister;
    uint16_t    baudRegister;

    //Internal Helper Variables
    uint32_t    baudCounter;    //Calculated value for baudrate timing [(baudRegister x Multiplier) / 2]

    //Joy RX FIFO
	lite::fifo<uint8_t, 8>	rxFifo;   

    //Joy TX Data
    uint32_t    txData;
};