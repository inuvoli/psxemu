#pragma once

#include <cstdint>
#include <cstdio>
#include <string>

#include "psx.h"

//#define GLEW_STATIC
#include <GL/glew.h>
#include <SDL.h>

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include "imgui_memory_editor.h"



class psxemu
{
public:
	psxemu();
	~psxemu();

	bool init(int wndWidth, int wndHeight);
	bool run();


public:
	//Emulator Status
	bool	isRunning;
	bool	instructionStep;
	bool	frameStep;

	//Debug Windows
	bool		showRegister;
	bool		showRom;
	bool		showRam;
	bool		showAssembler;
	bool		showDma;
	bool		showTimers;
	bool		showGpu;
	uint32_t	breakPoint;
	
	Psx			*pPsxSystem = nullptr;

private:
	bool handleEvents();
	bool debugInfo();
	bool update();
	bool updateFrame();
	bool renderFrame();

private:
	SDL_Event			sdlEvent;
	SDL_Window*			pWindow;
	SDL_GLContext		glContext;
	SDL_AudioDeviceID	sdlAudioDevice;

	MemoryEditor		dbgRom;
	MemoryEditor		dbgRam;
	MemoryEditor		dbgVRam;

	//Controllers
	SDL_GameController* pControllerA;
	SDL_GameController* pControllerB;

	//Performance Timers
	uint64_t	timerStart;
	uint64_t	timerStop;
	uint16_t	framePerSecond;

	//Debug Helper
	ImVec4 clear_color = ImVec4(0.15f, 0.25f, 0.30f, 1.00f);
	ImVec4 yellow_color = ImVec4(0.90f, 0.90f, 0.00f, 1.00f);
	ImVec4 green_color = ImVec4(0.00f, 0.90f, 0.00f, 1.00f);
	ImVec4 grey_color = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	ImVec4 darkgrey_color = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	ImVec4 red_color = ImVec4(0.90f, 0.00f, 0.00f, 1.00f);
	ImVec4 black_color = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);

	std::string cpuRegisterName[32] = { "zr", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
										"t8", "t9", "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "k0", "k1", "gp", "sp", "fp", "ra"};

	std::string cop0RegisterName[32] = { "", "", "", "BPC", "", "BDA", "JDEST", "DCIC", "BadVaddr", "BDAM", "", "BPCM", "SR",
										"Cause", "EPC", "PDId", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "" };
};

