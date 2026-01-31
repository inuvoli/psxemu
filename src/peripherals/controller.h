#pragma once

#include <cstdio>
#include <vector>
#include <memory>

#include "litelib.h"

constexpr auto CONTROLLER_DEADZONE = 10;            //Analog Stick Deadzone
constexpr auto ACK_SIGNAL_DURATION = 100;           //ACK Signal Duration in CPU Clock Ticks (about 3us)
constexpr auto CMD_TRANSMISSION_DURATION = 8;       //Command Transmission Duration in Controller Clock CONTROTicks

enum class ControllerButton : uint8_t
{
    SELECT = 0x0,
    L3 = 0x1,
    R3 = 0x2,
    START = 0x3,
    UP = 0x4,
    RIGHT = 0x5,
    DOWN = 0x6,
    LEFT = 0x7,
    L2 = 0x8,
    R2 = 0x9,
    L1 = 0xa,
    R1 = 0xb,
    TRIANGLE = 0xc,
    CIRCLE = 0xd,
    CROSS = 0xe,
    SQUARE = 0xf
};

enum class ControllerState : uint8_t
{
    IDLE = 0x0,
    SESSION_STARTED = 0x1,
    START_SEND_CMD_TO_PAD = 0x2,
    SENDING_CMD_TO_PAD = 0x3,
    START_RECV_RX_FROM_PAD = 0x4,
    END_RECV_RX_FROM_PAD = 0x5
};

enum class ControllerEvent : uint8_t
{
    START_TX_SESSION_EVENT = 0x0,
    END_TX_SESSION_EVENT = 0x1,
    TX_COMMAND_RECEIVED_EVENT = 0x2,  //Requires Received Command Code
    CONTROLLER_CLOCK_TICK_EVENT = 0x3,
    RX_RESPONSE_SENT_EVENT = 0x4,
    CPU_CLOCK_TICK_EVENT = 0x5
};

namespace controller
{
    union StatusRegister
    {
        uint32_t word;

        lite::bitfield<0, 1>   txready1;		//TX Ready Flag 1   (0=Busy, 1=Ready) [Monitor Transmission from BIOS to PSX]
        lite::bitfield<1, 1>   rxready;	        //RX FIFO Not Empty (0=No Data Available, 1=Data Available)
        lite::bitfield<2, 1>   txready2;		//TX Ready Flag 2   (0=sending data to Pad 1=Ready) [Monitor transmission from PSX to PAD] 
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

    struct State
    {
        uint16_t buttons;                        //Button States
        uint16_t analogR;                        //Right Analog Stick X Axis
        uint16_t analogL;                        //Left Analog Stick X Axis
        uint16_t analogT;                        //Analog Triggers
        bool     enabled;                        //Controller Enabled Flag, set when selected via CTRL_REGISTER.Bit1 and CTRL_REGISTER.Bit13
    };
};

class Psx;
class ControllerStateMachine;

class Controller
{
    friend class ControllerStateMachine;  // Allow state machine to access private members

public:
    Controller();
    ~Controller();

    bool execute();
    bool reset();

    bool writeAddr(uint32_t addr, uint32_t& data, uint8_t bytes);
	uint32_t readAddr(uint32_t addr, uint8_t bytes);

	void setButtonState(uint8_t controllerID, ControllerButton buttonMask, bool pressed);
	void setLeftAnalogStickX(uint8_t controllerID, int x);
	void setLeftAnalogStickY(uint8_t controllerID, int y);
	void setRightAnalogStickX(uint8_t controllerID, int x);
	void setRightAnalogStickY(uint8_t controllerID, int y);
	void setLeftTrigger(uint8_t controllerID, int value);
	void setRightTrigger(uint8_t controllerID, int value);

    //Connect to PSX Instance
	void link(Psx* instance) { psx = instance; }

private:
    //Link to PSP Object
	Psx* psx;

    //Internal Registers
    controller::StatusRegister                  statRegister;
    controller::ModeRegister                    modeRegister;
    controller::ControlRegister                 ctrlRegister;
    uint16_t                                    baudRegister;
    
	//Controller Button and Analog States
	controller::State	                        controllerState[2];

    //Internal Helper Variables
    uint32_t                                    baudCounter;        //Calculated value for baudrate timing [(baudRegister x Multiplier)]

    //Joy RX FIFO
	lite::fifo<uint8_t, 8>	rxFifo;
    
    //Joy TX Latch
    uint8_t     txLatch;

    //Controller State Machine
    std::unique_ptr<ControllerStateMachine>	    stateMachine;
};

//-------------------------------------------------------------------------------------------------------------
//
// Controller State Machine Declaration
//
//-------------------------------------------------------------------------------------------------------------
class ControllerStateMachine
{
public:
    ControllerStateMachine(Controller* ptr);
    ~ControllerStateMachine();

    void pushEvent(ControllerEvent event, uint8_t eventData = 0);
    void loadCommandResponse();

private:
    std::shared_ptr<Controller>     controller;
    ControllerState                 currentState;
};