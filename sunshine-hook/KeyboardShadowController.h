#pragma once
#include <Windows.h>
#include "InputCommand.h"
#include <map>
#include <queue>

struct Key
{
	ButtonEventType ev;
	USHORT scancode;
};

class KeyboardShadowController
{
public:
	std::map<UINT, ButtonEventType> keys;
	//std::queue<InputCommand> _commandQueue;
	void UpdateState(InputCommand command, HWND * wnd, int wndCount);
	int ReadInput(InputCommand commandBuffer[], size_t size);
	
	InputCommand GetInputCommand(HRAWINPUT hRawInput);
	static KeyboardShadowController* GetInstance()
	{
		if (instance == nullptr) {
			instance = new KeyboardShadowController();
		}
		return instance;
	}
	int lastCommand = 0;
private:
	static KeyboardShadowController * instance;
	InputCommand commands[128];
};

