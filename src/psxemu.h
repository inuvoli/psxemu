#pragma once

#include <cstdint>
#include <cstdio>
#include <string>
#include <memory>

#include "videolib.h"
#include "psx.h"
#include "debugger.h"

class psxemu
{
public:
	psxemu();
	~psxemu();

	bool init(int wndWidth, int wndHeight, const std::string& biosFileName, const std::string& gameFileName);
	bool run();


public:
	//Emulator Status
	bool	isRunning;
	std::shared_ptr<Psx>	pPsx;

private:
	bool handleEvents();
	bool debugInfo();
	bool update(StepMode stepMode);
	bool render(StepMode stepMode);
	bool renderWidgets();

private:
	SDL_Event			sdlEvent;
	SDL_Window*			pWindow;
	SDL_Window*			pWindow2;
	SDL_GLContext		glContext;
	SDL_AudioDeviceID	sdlAudioDevice;

	//Controllers
	SDL_GameController* pControllerA;
	SDL_GameController* pControllerB;
	
	//Performance Timers
	uint64_t	timerStart;
	uint64_t	timerStop;
	uint16_t	framePerSecond;

	//Debugger
	std::shared_ptr<Debugger>	pDebugger;

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
	
	//Debug Helper
	MipsDisassembler	mipsDisassembler;
	AsmCode asmCode;
	GLuint vramTexture;
	MemoryEditor		dbgRom;
	MemoryEditor		dbgRam;
	MemoryEditor		dbgVRam;
	
	ImVec4 yellow_color = ImVec4(0.90f, 0.90f, 0.00f, 1.00f);
	ImVec4 green_color = ImVec4(0.00f, 0.90f, 0.00f, 1.00f);
	ImVec4 grey_color = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	ImVec4 darkgrey_color = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	ImVec4 red_color = ImVec4(0.90f, 0.00f, 0.00f, 1.00f);
	ImVec4 black_color = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);

	//Registers names
	std::string cpuRegisterName[32] = { "zr", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
 										"t8", "t9", "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "k0", "k1", "gp", "sp", "fp", "ra"};

	std::string cop0RegisterName[32] = { "", "", "", "BPC", "", "BDA", "JDEST", "DCIC", "BadVaddr", "BDAM", "", "BPCM", "SR",
 										"Cause", "EPC", "PDId", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "" };

};

