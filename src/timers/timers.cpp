#include <loguru.hpp>
#include "timers.h"
#include "psx.h"
#include "gpu.h"

Timers::Timers()
{
	//Reset TIMERS Registers
	for (auto &e : timerStatus)
	{
		e.counterValue = 0x0;
		e.counterTarget = 0x0;
		e.counterMode.word = 0x0;
	}

	//Init Pulse Duration Counters
	pulseDuration[0] = PULSE_DURATION;
	pulseDuration[1] = PULSE_DURATION;
	pulseDuration[2] = PULSE_DURATION;
}

Timers::~Timers()
{
}

bool Timers::reset()
{
	//Reset TIMERS Registers
	for (auto &e : timerStatus)
	{
		e.counterValue = 0x0;
		e.counterTarget = 0x0;
		e.counterMode.word = 0x0;
	}

	return true;
}

bool Timers::execute(ClockSource source)
{
	//Update Timers according to Clock Source

	switch (source)
	{
	case ClockSource::System:
		if (timerStatus[0].clockSource == ClockSource::System) updateTimer0();
		if (timerStatus[1].clockSource == ClockSource::System) updateTimer1();
		if (timerStatus[2].clockSource == ClockSource::System) updateTimer2();
		break;

	case ClockSource::System8:
		if (timerStatus[2].clockSource == ClockSource::System8) updateTimer2();
		break;

	case ClockSource::Dot:
		if (timerStatus[0].clockSource == ClockSource::Dot) updateTimer0();
		break;

	case ClockSource::hBlank:
		if (timerStatus[1].clockSource == ClockSource::hBlank) updateTimer1();
		break;
		
	default:
		LOG_F(ERROR, "TIMERS - Unknown Clock Source!");
	}

	//Update Interrupt Signals
	updateInterrupt(0);
	updateInterrupt(1);
	updateInterrupt(2);

	return true;
}

void Timers::updateTimer0()
{
	//UPDATE Counter 0
	if ((bool)timerStatus[0].counterMode.syncEn)
	{
		//Sync Enable
		// - Counter is update according to selected clock source
		//			0 or 2 = System Clock
		//			1 or 3 = Dot Clock
		// - Counter is Synced according to Sync Mode value:
		//			0 = Pause counter during hBlank(s)
		//			1 = Reset counter to 0000h at hBlank(s)
		//			2 = Reset counter to 0000h at hBlank(s) and pause outside of hBlank
		//			3 = Pause until hBlank(s) occurs once, then switch to Free Run
		//  - Counter is reset to 0x0000 according to Reset Zero setting:
		//			0 = after counter reach 0xfffff, 1 = after counter reach Counter Target
		//
		switch (timerStatus[0].counterMode.syncMode)
		{
		case 0:
			if (!psx->gpu->hBlank) timerStatus[0].counterValue++;
			break;
		case 1:
			timerStatus[0].counterValue++;
			if (psx->gpu->hBlank) timerStatus[0].counterValue = 0x0000;
			break;
		case 2:
			if (psx->gpu->hBlank) timerStatus[0].counterValue++; ///DA VERIFICARE, NON HA SENSO
			if (psx->gpu->hBlank) timerStatus[0].counterValue = 0x0000; //CHECK - hBlank should be active only at the end of visible area?
			break;
		case 3:
			if (psx->gpu->hBlank) timerStatus[0].counterMode.syncEn = false;
			break;

		default:
			LOG_F(ERROR, "TIMER - Unknown Sync Mode!");
		}

		if (static_cast<bool>(timerStatus[0].counterMode.resetZero))
		{
			if (timerStatus[0].counterValue > timerStatus[0].counterTarget)
			{
				timerStatus[0].counterValue = 0x0000;
				timerStatus[0].counterTarget = true;
			}
		}
		else
		{
			if (timerStatus[0].counterValue > 0xffff)
			{
				timerStatus[0].counterValue = 0x0000;
				timerStatus[0].toOverflow = true;
			}
		}
	}
	else
	{
		//Sync Disabled (aka Free Run)
		// - Counter is update according to selected clock source
		//			0 or 2 = System Clock
		//			1 or 3 = Dot Clock
		// - Counter if always reset when reach 0xffff
		timerStatus[0].counterValue++;
		if (timerStatus[0].counterValue > 0xffff)
		{
			timerStatus[0].counterValue = 0x0000;
			timerStatus[0].toOverflow = true;
		}
	}
}

void Timers::updateTimer1()
{
	//UPDATE Counter 1
	if ((bool)timerStatus[1].counterMode.syncEn)
	{
		//Sync Enable
		// - Counter is update according to selected clock source
		//			0 or 2 = System Clock
		//			1 or 3 = hBlank
		// - Counter is Synced according to Sync Mode value:
		//			0 = Pause counter during vBlank(s)
		//			1 = Reset counter to 0000h at vBlank(s)
		//			2 = Reset counter to 0000h at vBlank(s) and pause outside of vBlank
		//			3 = Pause until vBlank(s) occurs once, then switch to Free Run
		//  - Counter is reset to 0x0000 according to Reset Zero setting:
		//			0 = after counter reach 0xfffff, 1 = after counter reach Counter Target
		//
		switch (timerStatus[1].counterMode.syncMode)
		{
		case 0:
			if (!psx->gpu->vBlank) timerStatus[1].counterValue++;
			break;
		case 1:
			timerStatus[1].counterValue++;
			if (psx->gpu->vBlank) timerStatus[1].counterValue = 0x0000;
			break;
		case 2:
			if (psx->gpu->vBlank) timerStatus[0].counterValue++; ///DA VERIFICARE, NON HA SENSO
			if (psx->gpu->vBlank) timerStatus[1].counterValue = 0x0000; //CHECK - vBlank shoult be active only at the end of visible area?
			break;
		case 3:
			if (psx->gpu->vBlank) timerStatus[1].counterMode.syncEn = false;
			break;

		default:
			LOG_F(ERROR, "TIMER - Unknown Sync Mode!");
		}

		if (static_cast<bool>(timerStatus[1].counterMode.resetZero))
		{
			if (timerStatus[1].counterValue > timerStatus[1].counterTarget)
			{
				timerStatus[1].counterValue = 0x0000;
				timerStatus[1].counterTarget = true;
			}
		}
		else
		{
			if (timerStatus[1].counterValue > 0xffff)
			{
				timerStatus[1].counterValue = 0x0000;
				timerStatus[1].toOverflow = true;
			}
		}
	}
	else
	{
		//Sync Disabled (aka Free Run)
		// - Counter is update according to selected clock source
		//			0 or 2 = System Clock
		//			1 or 3 = hBlank
		// - Counter if always reset when reach 0xffff
		timerStatus[1].counterValue++;
		if (timerStatus[1].counterValue > 0xffff)
		{
			timerStatus[1].counterValue = 0x0000;
			timerStatus[1].toOverflow = true;
		}
	}
}

void Timers::updateTimer2()
{
	//UPDATE Counter 2
	if ((bool)timerStatus[2].counterMode.syncEn)
	{
		//Sync Enable
		// - Counter is update according to selected clock source
		//			0 or 1 = System Clock
		//			2 or 3 = System Clock / 8
		// - Counter is Synced according to Sync Mode value:
		//			0 or 3 = Stop counter at current value (forever, no h/v-blank start)
		//			1 or 2 = Free Run (same as when Synchronization Disabled)
		//  - Counter is reset to 0x0000 according to Reset Zero setting:
		//			0 = after counter reach 0xfffff, 1 = after counter reach Counter Target
		//
		switch (timerStatus[2].counterMode.syncMode)
		{
		case 0:
		case 3:
			break;

		case 1:
		case 2:
			timerStatus[2].counterValue++;
			break;
		
		default:
			LOG_F(ERROR, "TIMER - Unknown Sync Mode!");
		}

		if (static_cast<bool>(timerStatus[2].counterMode.resetZero))
		{
			if (timerStatus[2].counterValue > timerStatus[2].counterTarget)
			{
				timerStatus[2].counterValue = 0x0000;
				timerStatus[2].counterTarget = true;
			}
		}
		else
		{
			if (timerStatus[2].counterValue > 0xffff)
			{
				timerStatus[2].counterValue = 0x0000;
				timerStatus[2].toOverflow = true;
			}
		}
	}
	else
	{
		//Sync Disabled (aka Free Run)
		// - Counter is update according to selected clock source
		//			0 or 1 = System Clock
		//			2 or 3 = System Clock / 8
		// - Counter if always reset when reach 0xffff
		timerStatus[2].counterValue++;
		if (timerStatus[2].counterValue > 0xffff)
		{
			timerStatus[2].counterValue = 0x0000;
			timerStatus[2].toOverflow = true;
		}
	}
}

bool Timers::writeAddr(uint32_t addr, uint32_t& data, uint8_t bytes)
{
	// Counter Mode data bit meaning:
	// 0	Synchronization Enable(0 = Free Run, 1 = Synchronize via Bit1 - 2)
	// 1-2	Synchronization Mode(0 - 3, see lists below)
	//		Synchronization Modes for Counter 0:
	//			- 0 = Pause counter during Hblank(s)
	//			- 1 = Reset counter to 0000h at Hblank(s)
	//			- 2 = Reset counter to 0000h at Hblank(s) and pause outside of Hblank
	//			- 3 = Pause until Hblank occurs once, then switch to Free Run
	//		Synchronization Modes for Counter 1 :
	//			- Same as above, but using Vblank instead of Hblank
	//		Synchronization Modes for Counter 2 :
	//			- 0 or 3 = Stop counter at current value(forever, no h / v - blank start)
	//			- 1 or 2 = Free Run(same as when Synchronization Disabled)
	//	3     Reset counter to 0000h(0 = After Counter = FFFFh, 1 = After Counter = Target)
	//	4     IRQ when Counter = Target(0 = Disable, 1 = Enable)
	//	5     IRQ when Counter = FFFFh(0 = Disable, 1 = Enable)
	//	6     IRQ Once / Repeat Mode(0 = One - shot, 1 = Repeatedly)
	//	7     IRQ Pulse / Toggle Mode(0 = Short Bit10 = 0 Pulse, 1 = Toggle Bit10 on / off)
	//	8-9   Clock Source(0 - 3, see list below)
	//			- Counter 0:  0 or 2 = System Clock, 1 or 3 = Dotclock
	//			- Counter 1 : 0 or 2 = System Clock, 1 or 3 = Hblank
	//			- Counter 2 : 0 or 1 = System Clock, 2 or 3 = System Clock / 8
	//	10    Interrupt Request(0 = Yes, 1 = No) (Set after Writing)    (W = 1) (R)
	//	11    Reached Target Value(0 = No, 1 = Yes) (Reset after Reading)        (R)
	//	12    Reached FFFFh Value(0 = No, 1 = Yes) (Reset after Reading)        (R)
	
	switch (addr)
	{
		//--------------------------------------------Timer 0
	case 0x1f801100:	//----------------------------Counter Value 0
		timerStatus[0].counterValue = data & 0x0000ffff;
		break;

	case 0x1f801104:	//----------------------------Counter Mode 0
		timerStatus[0].counterMode.word = data & 0x0000ffff;
		timerStatus[0].counterValue = 0x00000000;		//Counter Value is forcefully reset to 0x0000 on any write to Counter Mode 
		timerStatus[0].counterMode.irqRequest = true;	//Interrupt Request is set at every Write (false = Yes, true = No)

		//Set Clock Source Helper Variable
		if (timerStatus[0].counterMode.clkSource == 0 || timerStatus[0].counterMode.clkSource == 2)
			timerStatus[0].clockSource = ClockSource::System;
		else
			timerStatus[0].clockSource = ClockSource::Dot;
		
		timerStatus[0].toTarget = false;
		timerStatus[0].toOverflow = false;
		break;

	case 0x1f801108:	//----------------------------Counter Target 0
		timerStatus[0].counterTarget = data & 0x0000ffff;
		break;

		//--------------------------------------------Timer 1
	case 0x1f801110:	//----------------------------Counter Value 1
		timerStatus[1].counterValue = data & 0x0000ffff;
		break;

	case 0x1f801114:	//----------------------------Counter Mode 1
		timerStatus[1].counterMode.word = data & 0x0000ffff;
		timerStatus[1].counterValue = 0x00000000;		//Counter Value is forcefully reset to 0x0000 on any write to Counter Mode 
		timerStatus[1].counterMode.irqRequest = true;	//Interrupt Request is set at every Write (false = Yes, true = No)

		//Set Clock Source Helper Variable
		if (timerStatus[1].counterMode.clkSource == 0 || timerStatus[1].counterMode.clkSource == 2)
			timerStatus[1].clockSource = ClockSource::System;
		else
			timerStatus[1].clockSource = ClockSource::hBlank;
		
		timerStatus[1].toTarget = false;
		timerStatus[1].toOverflow = false;
		break;

	case 0x1f801118:	//----------------------------Counter Target 1
		timerStatus[1].counterTarget = data & 0x0000ffff;
		break;

		//-------------------------------Timer 2
	case 0x1f801120:	//----------------------------Counter Value 2
		timerStatus[2].counterValue = data & 0x0000ffff;
		break;

	case 0x1f801124:	//----------------------------Counter Mode 2
		timerStatus[2].counterMode.word = data & 0x0000ffff;
		timerStatus[2].counterValue = 0x00000000;		//Counter Value is forcefully reset to 0x0000 on any write to Counter Mode 
		timerStatus[2].counterMode.irqRequest = true;	//Interrupt Request is set at every Write (false = Yes, true = No)

		//Set Clock Source Helper Variable
		if (timerStatus[2].counterMode.clkSource == 0 || timerStatus[2].counterMode.clkSource == 1)
			timerStatus[2].clockSource = ClockSource::System;
		else
			timerStatus[2].clockSource = ClockSource::System8;
		
		timerStatus[2].toTarget = false;
		timerStatus[2].toOverflow = false;
		break;

	case 0x1f801128:	//----------------------------Counter Target 2
		timerStatus[1].counterTarget = data & 0x0000ffff;
		break;

	default:
		LOG_F(ERROR, "TIMERS - Unknown Parameter Set addr: 0x%08x (%d), data: 0x%08x", addr, bytes, data);
		return false;
	}
	return true;
}

uint32_t Timers::readAddr(uint32_t addr, uint8_t bytes)
{
	uint32_t data;

	switch (addr)
	{
		//-------------------------------Timer 0
	case 0x1f801100:
		data = timerStatus[0].counterValue;
		break;

	case 0x1f801104:
		data = timerStatus[0].counterMode.word;
		break;

	case 0x1f801108:
		data = timerStatus[0].counterTarget;
		break;

		//-------------------------------Timer 1
	case 0x1f801110:
		data = timerStatus[1].counterValue;
		break;

	case 0x1f801114:
		data = timerStatus[1].counterMode.word;
		break;

	case 0x1f801118:
		data = timerStatus[1].counterTarget;
		break;

		//-------------------------------Timer 2
	case 0x1f801120:
		data = timerStatus[2].counterValue;
		break;

	case 0x1f801124:
		data = timerStatus[2].counterMode.word;
		break;

	case 0x1f801128:
		data = timerStatus[2].counterTarget;
		break;

	default:
		LOG_F(ERROR, "TIMERS - Unknown Parameter Get addr: 0x%08x (%d)", addr, bytes);
		return 0x0;
	}

	return data;
}

void Timers::updateInterrupt(uint8_t timerNumber)
{
	bool toggleMode = (bool)timerStatus[timerNumber].counterMode.irqToggle;
	bool repeatMode = (bool)timerStatus[timerNumber].counterMode.irqRepeat;
	bool isTarget = (bool)timerStatus[timerNumber].counterMode.isTarget;
	bool isOverflow = (bool)timerStatus[timerNumber].counterMode.isOverflow;
	bool triggerTarget = timerStatus[timerNumber].toTarget;
	bool triggerOverflow = timerStatus[timerNumber].toOverflow;
	bool throwInterrupt = false;

	if (toggleMode)
	{
		//Toggle Mode
		if (repeatMode)
		{
			//Repeat Mode
			if (isTarget & triggerTarget)
			{
				timerStatus[timerNumber].counterMode.irqRequest = (bool)(timerStatus[timerNumber].counterMode.irqRequest) ? false : true;
				timerStatus[timerNumber].toTarget = false;
				throwInterrupt = !(bool)timerStatus[timerNumber].counterMode.irqRequest;
			}

			if (isOverflow & triggerOverflow)
			{
				timerStatus[timerNumber].counterMode.irqRequest = (bool)(timerStatus[timerNumber].counterMode.irqRequest) ? false : true;
				timerStatus[timerNumber].toOverflow = false;
				throwInterrupt = !(bool)timerStatus[timerNumber].counterMode.irqRequest;
			}
				
		}
		else
		{
			//Once Mode
			if (isTarget & triggerTarget)
			{
				timerStatus[timerNumber].counterMode.irqRequest = false;
				timerStatus[timerNumber].toTarget = false;
				throwInterrupt = true;
			}

			if (isOverflow & triggerOverflow)
			{
				timerStatus[timerNumber].counterMode.irqRequest = false;
				timerStatus[timerNumber].toOverflow = false;
				throwInterrupt = true;
			}
		}
	}
	else
	{
		//Pulse Mode
		if (repeatMode)
		{
			if (timerStatus[timerNumber].counterMode.irqRequest = false)
			{
				pulseDuration[timerNumber]--;
				if (pulseDuration == 0)
					timerStatus[timerNumber].counterMode.irqRequest = true;
				return;
			}

			//Repeat Mode
			if (isTarget & triggerTarget)
			{
				timerStatus[timerNumber].counterMode.irqRequest = false;
				pulseDuration[timerNumber] = PULSE_DURATION;
				timerStatus[timerNumber].toTarget = false;
				throwInterrupt = true;
			}

			if (isOverflow & triggerOverflow)
			{
				timerStatus[timerNumber].counterMode.irqRequest = false;
				pulseDuration[timerNumber] = PULSE_DURATION;
				timerStatus[timerNumber].toOverflow = false;
				throwInterrupt = true;
			}
		}
		else
		{
			if (timerStatus[timerNumber].counterMode.irqRequest = false)
			{
				pulseDuration[timerNumber]--;
				if (pulseDuration == 0)
					timerStatus[timerNumber].counterMode.irqRequest = true;
				//Hack
				timerStatus[timerNumber].counterMode.isTarget = false;
				timerStatus[timerNumber].counterMode.isOverflow = false;

				return;
			}

			//Once Mode
			if (isTarget & triggerTarget)
			{
				timerStatus[timerNumber].counterMode.irqRequest = false;
				pulseDuration[timerNumber] = PULSE_DURATION;
				timerStatus[timerNumber].toTarget = false;
				throwInterrupt = true;
			}

			if (isOverflow & triggerOverflow)
			{
				timerStatus[timerNumber].counterMode.irqRequest = false;
				pulseDuration[timerNumber] = PULSE_DURATION;
				timerStatus[timerNumber].toOverflow = false;
				throwInterrupt = true;
			}
		}
	}

	//Throw Interrupt
	if (throwInterrupt)
	{
		switch(timerNumber)
		{
		case 0:
			psx->interrupt->set(static_cast<uint32_t>(interruptCause::timer0));
			break;
		case 1:
			psx->interrupt->set(static_cast<uint32_t>(interruptCause::timer1));
			break;
		case 2:
			psx->interrupt->set(static_cast<uint32_t>(interruptCause::timer2));
			break;
			
		}

		throwInterrupt = false;
	}
}
