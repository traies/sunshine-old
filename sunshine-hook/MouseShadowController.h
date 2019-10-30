#pragma once
#include "InputCommand.h"

class MouseShadowController
{
public:
	bool lb, rb, mb, x1b, x2b;
	long lastX = 0, lastY = 0, lastXRelative = 0, lastYRelative = 0;
	USHORT scrollOffset;

	void UpdateState(InputCommand command, HWND * wnd, int windowsCount);

	static MouseShadowController * GetInstance()
	{
		if (instance == nullptr) {
			instance = new MouseShadowController();
		}
		return instance;
	}
private:
	static MouseShadowController * instance;
};

