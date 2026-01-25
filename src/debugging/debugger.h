#pragma once
//#define DEBUGGER_ENABLED

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
struct KernelCallEvent
{
	uint32_t		pc;		//Program Counter (Address of the Kernel Call)
	uint32_t		sp;		//Stack Pointer
	uint32_t		ra;		//Return Address
	uint32_t		t1;		//Function Identifier
	uint32_t		a0;		//Argument 1
	uint32_t		a1;		//Argument 2
	uint32_t		a2;		//Argument 3
	uint32_t		a3;		//Argument 4
};

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
	lite::vec2t<uint16_t>		displayStart; 
	lite::vec4t<uint16_t>		displayRange; 
	lite::vec2t<uint16_t>		drawingOffset; 
	lite::vec4t<uint16_t>		drawingArea; 
	lite::vec2t<uint16_t>		videoResolution; 
	std::string					videoStandard; 
	std::string					textureDisabled;
	lite::vec2t<uint16_t>		texturePage;
	std::string					colorMode;
	lite::vec2t<uint8_t>		textureMask;
	lite::vec2t<uint8_t>		textureOffset;
	bool						displayDisabled;
	bool						interlaced;
};

struct CdromDebugInfo
{
	uint8_t		statusRegister;
	uint8_t		requestRegister;
	uint8_t		interruptMaskRegister;
	uint8_t		interruptStatusRegister;
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

class Debugger
{
public:
	static Debugger& instance()
    {
        static Debugger *instance = new Debugger();
        return *instance;
    }

	//Debugger Interface
	static bool init();
	static bool update();
    static bool render();

	//Connect to PSX Instance
	static void link(Psx* psxInstance) { instance().psx = psxInstance; }

	//Debug Controls
	static bool isBreakpoint (); 
    static void setBreakpoint(uint32_t addr) { instance().breakPoint = addr; };
    static uint32_t getBreakPoint() { return instance().breakPoint; };
    static void setStepMode(StepMode mode) { instance().stepMode = mode; };
    static StepMode getStepMode() { return instance().stepMode; };
	static void setFrameRate(uint16_t framerate) { instance().framePerSecond = framerate; };
	static uint16_t getFrameRate() { return instance().framePerSecond; };
    static void toggleDebugModuleStatus(DebugModule module);
    static bool getDebugModuleStatus(DebugModule module);
	static void dumpRam();

	//Kernel Call Callback
	void getCallStackInfo(KernelCallEvent &e);

private:
	Debugger() {}

	//Debug Info
	void getInterruptDebugInfo();
	void getTimerDebugInfo();
	void getGpuDebugInfo();
	void getCdromDebugInfo();
	
	//Render Widgets Functions
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

private:
	//Link to Bus Object
    Psx* psx;

	//VRAM Texture Debug Copy
	GLuint				vramDebugTexture;

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
	MemoryEditor		dbgRom;
	MemoryEditor		dbgRam;
	MemoryEditor		dbgVRam;
	
	ImVec4 yellow_color = ImVec4(0.90f, 0.90f, 0.00f, 1.00f);
	ImVec4 green_color = ImVec4(0.00f, 0.90f, 0.00f, 1.00f);
	ImVec4 grey_color = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	ImVec4 darkgrey_color = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	ImVec4 red_color = ImVec4(0.90f, 0.00f, 0.00f, 1.00f);
	ImVec4 black_color = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	ImVec4 white_color = ImVec4(1.0f, 1.00f, 1.00f, 1.00f);

	//Debug State names
	std::string debugStates[4] = { "Halt", "Manual", "Instruction", "Frame"};
	
	// UI flags
	bool openSetBreakpointPopup = false;
};