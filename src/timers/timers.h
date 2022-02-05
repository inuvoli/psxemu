#pragma once

#include <cstdint>
#include <cstdio>
#include <string>
#include <array>
#include <memory>

#include "bitfield.h"

class Psx;

constexpr auto TIMER_NUMBER = 3;

enum class ClockSource { System, System8, Dot, hBlank };

union CounterMode
{
	uint32_t		word;

	BitField<0, 1>	syncEn;			//Syncronization Enable (0 = Free Run, 1 = Syncronize via Mode)
	BitField<1, 2>	syncMode;		//Syncronyzation Mode
	BitField<3, 1>	resetZero;		//0 = Reset to 0 after counter reach 0xffff, 1 = Reset to 0 after counter reach target
	BitField<4, 1>	irqTarget;		//Enable IRQ when Counter reach Target
	BitField<5, 1>	irqOverflow;	//Enable IRQ when Counter reach 0xffff
	BitField<6, 1>	irqRepeat;		//Enable IRQ repeat. 0 = One Shot, 1 = Repeat
	BitField<7, 1>	irqToggle;		//0 = Short Pulse of bit 10, 1 = Toggle bit10 on and off
	BitField<8, 2>	clkSource;		//Depends on which Timer is used of the 3
	BitField<10, 1>	irqRequest;		//Enable IRQ Request
	BitField<11, 1> isTarget;		//Set to 1 if Counter has reached Target Value, Reset to 0 after Reading
	BitField<12, 1> isOverflow;		//Set to 1 if Counter has reached 0xffff Value, Reset to 0 after Reading
};

struct TimerStatus
{
	//Raw Register Values
	uint32_t	counterValue;	//Timer Counter Current Value (R/W)
	uint32_t	counterTarget;	//Timer Counter Target Value (R/W)
	CounterMode	counterMode;	//Timer Counter Mode (R/W)
	//Helper Variables
	ClockSource	clockSource;

	TimerStatus& operator=(TimerStatus& x)
	{
		this->counterValue = x.counterValue;
		this->counterTarget = x.counterTarget;
		this->counterMode = x.counterMode;
		this->clockSource = x.clockSource;

		return *this;
	}
};

//GPU Debug Status
struct TimerDebugInfo
{
	TimerStatus		timerStatus[TIMER_NUMBER];
};

class Timers
{
public:
	Timers();
	~Timers();

	bool reset();
	bool clock(ClockSource source);

	bool setParameter(uint32_t addr, uint32_t& data, uint8_t bytes);
	uint32_t getParameter(uint32_t addr, uint8_t bytes);

	//Connect to PSX Instance
	void link(Psx* instance) { psx = instance; }

	//Debug Info
	void getDebugInfo(TimerDebugInfo& info);
	
private:
	//Link to Bus Object
	Psx* psx;

	void updateTimer0();
	void updateTimer1();
	void updateTimer2();

	//Timers Internal Registers
	std::array<TimerStatus, TIMER_NUMBER>	timerStatus;
};

