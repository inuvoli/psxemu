#pragma once

#include <cstdint>
#include <cstdio>
#include <string>
#include <array>
#include <memory>

#include "litelib.h"

class Psx;

constexpr auto TIMER_NUMBER = 3;
constexpr auto PULSE_DURATION = 1500;

enum class ClockSource { System, System8, Dot, hBlank };

union CounterMode
{
	uint32_t		word;

	lite::bitfield<0, 1>	syncEn;			//Syncronization Enable (0 = Free Run, 1 = Syncronize via Mode)
	lite::bitfield<1, 2>	syncMode;		//Syncronyzation Mode
	lite::bitfield<3, 1>	resetZero;		//0 = Reset to 0 after counter reach 0xffff, 1 = Reset to 0 after counter reach target
	lite::bitfield<4, 1>	irqTarget;		//Enable IRQ when Counter reach Target
	lite::bitfield<5, 1>	irqOverflow;	//Enable IRQ when Counter reach 0xffff
	lite::bitfield<6, 1>	irqRepeat;		//Enable IRQ repeat. 0 = One Shot, 1 = Repeat
	lite::bitfield<7, 1>	irqToggle;		//0 = Short Pulse of bit 10, 1 = Toggle bit10 on and off
	lite::bitfield<8, 2>	clkSource;		//Depends on which Timer is used of the 3
	lite::bitfield<10, 1>	irqRequest;		//Enable IRQ Request
	lite::bitfield<11, 1> isTarget;		//Set to 1 if Counter has reached Target Value, Reset to 0 after Reading
	lite::bitfield<12, 1> isOverflow;		//Set to 1 if Counter has reached 0xffff Value, Reset to 0 after Reading
};

struct TimerStatus
{
	//Raw Register Values
	uint32_t	counterValue;	//Timer Counter Current Value (R/W)
	uint32_t	counterTarget;	//Timer Counter Target Value (R/W)
	CounterMode	counterMode;	//Timer Counter Mode (R/W)
	//Helper Variables
	ClockSource	clockSource;
	bool		toTarget;		//Timer Counter has reached Target Value
	bool		toOverflow;		//Timer Counter has overflowed (0xffff)

	TimerStatus& operator=(const TimerStatus& x)
	{
		this->counterValue = x.counterValue;
		this->counterTarget = x.counterTarget;
		this->counterMode = x.counterMode;
		this->clockSource = x.clockSource;
		this->toTarget = x.toTarget;
		this->toOverflow = x.toOverflow;

		return *this;
	}
};

class Timers
{
public:
	Timers();
	~Timers();

	bool reset();
	bool execute(ClockSource source);

	bool writeAddr(uint32_t addr, uint32_t& data, uint8_t bytes);
	uint32_t readAddr(uint32_t addr, uint8_t bytes);

	//Connect to PSX Instance
	void link(Psx* instance) { psx = instance; }

//#ifdef DEBUGGER_ENABLED
    //Getter & Setters
    TimerStatus getTimerStatus(int timerID) const { return timerStatus[timerID]; }
    void        setTimerStatus(int timerID, TimerStatus value) { timerStatus[timerID] = value; }
//#endif
	
private:
	//Link to Bus Object
	Psx* psx;

	void updateTimer0();
	void updateTimer1();
	void updateTimer2();
	void updateInterrupt(uint8_t timerNumber);

	uint32_t	pulseDuration[TIMER_NUMBER];

	//Timers Internal Registers
	std::array<TimerStatus, TIMER_NUMBER>	timerStatus;
};

