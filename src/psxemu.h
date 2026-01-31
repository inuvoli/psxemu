#pragma once

#include <cstdint>
#include <cstdio>
#include <string>
#include <memory>

#include "videolib.h"
#include "psx.h"

constexpr auto MINIMUM_SCREEN_WIDTH = 640;
constexpr auto MINIMUM_SCREEN_HEIGHT = 480;
constexpr auto MAX_GAMEPADS = 2;

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
	void updateGamepadsButtonsState(SDL_JoystickID id, int button, bool pressed) const;
	void updateGamepadsAxisMotion(SDL_JoystickID id, int axis, int value) const;

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
	int					numGamepads;
	SDL_JoystickID      GamepadIDs[MAX_GAMEPADS];
	SDL_Gamepad*		Gamepad[MAX_GAMEPADS];
};

