#pragma once
#include "InputCommand.h"

class MouseShadowController
{
public:
	bool lb, rb, mb, x1b, x2b;
	long lastX, lastY;
	USHORT scrollOffset;

	void UpdateState(InputCommand command, HWND wnd);

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

