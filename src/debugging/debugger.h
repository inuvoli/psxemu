#pragma once
#define DEBUGGER_ENABLED

#include <cstdint>
#include <memory>
#include <fstream>
#include <iostream>

#include "videolib.h"
#include "timers.h"
#include "vectors.h"
#include "mipsdisassembler.h"


constexpr auto ModulesNumber = 15;
enum class StepMode { Halt = 0, Manual = 1, Instruction = 2, Frame = 3 };
enum class DebugModule { Bios = 0, Ram = 1, Cpu = 2, Code = 3, Dma = 4, Timers = 5, Gpu = 6, Spu = 7, Cdrom = 8, Tty = 9};

//DebugInfo
struct InterruptDebugInfo
{
	uint32_t		i_stat;
	uint32_t		i_mask;
};

struct TimerDebugInfo
{
	TimerStatus		timerStatus[TIMER_NUMBER];
};

struct GpuDebugInfo
{
	uint32_t					gpuStat;
	void*						vRam;
	lite::vec2t<uint16_t>		displayStart; 
	lite::vec4t<uint16_t>		displayRange; 
	lite::vec2t<uint16_t>		drawingOffset; 
	lite::vec4t<uint16_t>		drawingArea; 
	lite::vec2t<uint16_t>		videoResolution; 
	std::string					videoStandard; 
	std::string					textureDisabled;
	uint16_t					texturePageYBase2;
	lite::vec2t<uint16_t>		texturePage;
	std::string					textureColorDepth;
	lite::vec2t<uint8_t>		textureMask;
	lite::vec2t<uint8_t>		textureOffset;
};

struct CdromDebugInfo
{
	uint8_t		statusRegister;
	uint8_t		requestRegister;
	uint8_t		interruptEnableRegister;
	uint8_t		interruptFlagRegister;
};

struct CallStackInfo
{
	uint32_t		pc;		//Program Counter value at the target address
	uint32_t		sp;		//Stack Pointer value at target address
	uint32_t		ra;		//Return Address value at target address
	std::string		func;	//Function Name
};

//Class Psx forward declaration
class Psx;

class debugger
{
public:
	static debugger& instance()
    {
        static debugger *instance = new debugger();
        return *instance;
    }

	bool isBreakpoint (); 
    void setBreakpoint(uint32_t addr) { breakPoint = addr; };
    uint32_t getBreakPoint() { return breakPoint; };

    void setStepMode(StepMode mode) { stepMode = mode; };
    StepMode getStepMode() { return stepMode; };

	void setFrameRate(uint16_t framerate) {framePerSecond = framerate;};
	uint16_t getFrameRate() {return framePerSecond;};

    void toggleDebugModuleStatus(DebugModule module);
    bool getDebugModuleStatus(DebugModule module);

	void dumpRam();

    bool update();
    bool render();

    //Debugger Widgets Functions
    bool renderFpsWidget();
	bool renderBiosWidget();
	bool renderRamWidget();
	bool renderCpuWidget();
	bool renderCodeWidget();
	bool renderDmaWidget();
	bool renderTimersWidget();
	bool renderGpuWidget();
	bool renderSpuWidget();
	bool renderCdromWidget();
	bool renderTtyWidget();
	bool renderMenuBar();

	//Connect to PSX Instance
	void link(Psx* psxInstance) { psx = psxInstance; }

private:
	debugger() {}

	//Debug Info
	void getInterruptDebugInfo();
	void getTimerDebugInfo();
	void getGpuDebugInfo();
	void getCdromDebugInfo();
	void updateCallStack();

private:
	//Link to Bus Object
    Psx* psx;

	//Debug Info
	lite::circularbuffer<CallStackInfo, 31>	callStack;		//Call Stack
	InterruptDebugInfo						interruptInfo;	//Interrupt Debug Info
	TimerDebugInfo							timerInfo;		//Timers Debug Info
	GpuDebugInfo							gpuInfo;		//GPU Debug Info
	CdromDebugInfo							cdromInfo;		//CDROM Debug Info
	
	//Debug Status
    bool                debugModuleStatus[ModulesNumber] = {};
    
	uint32_t	        breakPoint;
    StepMode            stepMode;
    uint16_t	        framePerSecond;

    //Debug Helper
	MipsDisassembler	mipsDisassembler;
	AsmCode             asmCode;
	GLuint              vramTexture;
	MemoryEditor		dbgRom;
	MemoryEditor		dbgRam;
	MemoryEditor		dbgVRam;
	
	ImVec4 yellow_color = ImVec4(0.90f, 0.90f, 0.00f, 1.00f);
	ImVec4 green_color = ImVec4(0.00f, 0.90f, 0.00f, 1.00f);
	ImVec4 grey_color = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	ImVec4 darkgrey_color = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	ImVec4 red_color = ImVec4(0.90f, 0.00f, 0.00f, 1.00f);
	ImVec4 black_color = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);

	//Debug State names
	std::string debugStates[4] = { "Halt", "Manual", "Instruction", "Frame"};
	
	// UI flags
	bool openSetBreakpointPopup = false;
};