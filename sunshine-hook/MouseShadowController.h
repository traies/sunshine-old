#pragma once
#include "InputCommand.h"
#include <queue>
#include <map>

class MouseShadowController
{
public:
	bool lb, rb, mb, x1b, x2b;
	long lastX = 0, lastY = 0;
	std::map<UINT, ButtonEventType> keys;
	USHORT scrollOffset;

	void UpdateState(InputCommand command, HWND * wnd, int windowsCount);

	static MouseShadowController * GetInstance()
	{
		if (instance == nullptr) {
			instance = new MouseShadowController();
		}
		return instance;
	}

	RAWMOUSE GetRawMouse(HRAWINPUT hRawInput);
	int lastCommand = 0;
private:
	static MouseShadowController * instance;
	RAWMOUSE commands[128];
	RAWMOUSE rawMouse;
};

