#pragma once

#include <cstdint>
#include <cstdio>
#include <string>
#include <memory>

#include "videolib.h"
#include "psx.h"

constexpr auto MINIMUM_SCREEN_WIDTH = 640;
constexpr auto MINIMUM_SCREEN_HEIGHT = 480;

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
	//Main Window Size
	int					windowWidth;
	int					windowHeight;

	//SDL Environment Objects
	SDL_Event			sdlEvent;
	SDL_Window*			pWindow;
	SDL_GLContext		glContext;
	SDL_AudioDeviceID	sdlAudioDevice;

	//Gamepads
	SDL_Gamepad* pGamepadA;
	SDL_Gamepad* pGamepadB;
};

