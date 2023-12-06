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

private:
	SDL_Event			sdlEvent;
	SDL_Window*			pWindow;
	SDL_GLContext		glContext;
	SDL_AudioDeviceID	sdlAudioDevice;

	//Controllers
	SDL_GameController* pControllerA;
	SDL_GameController* pControllerB;
		
	//Debugger
	std::shared_ptr<debugger>	pDebugger;
};

