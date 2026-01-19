#pragma once

#include <cstdint>
#include <cstdio>
#include <string>
#include <memory>

#include "videolib.h"
#include "psx.h"


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
	std::shared_ptr<Psx>	psx;

private:
	bool handleEvents();
	bool update(StepMode stepMode);
	bool update();
	bool render();

private:
	SDL_Event			sdlEvent;
	SDL_Window*			pWindow;
	SDL_GLContext		glContext;
	SDL_AudioDeviceID	sdlAudioDevice;

	//Gamepads
	SDL_Gamepad* pGamepadA;
	SDL_Gamepad* pGamepadB;
};

