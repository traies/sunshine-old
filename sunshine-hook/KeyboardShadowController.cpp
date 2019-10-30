#include "stdafx.h"
#include "KeyboardShadowController.h"
#include "WndProcHook.h"

KeyboardShadowController* KeyboardShadowController::instance = nullptr;

void KeyboardShadowController::UpdateState(InputCommand command, HWND * wnd, int windowsCount)
{
	if (command.event1 == 0) {
		return;
	}
	UINT message = 0;
	LPARAM lparam = 0;
	UINT key = MapVirtualKeyA(command.val2, MAPVK_VSC_TO_VK_EX);
	command.val1 = key;
	if (command.event1 == (int16_t)ButtonEventType::BUTTON_EVENT_DOWN) {
		message = DV_KEYDOWN;
		WORD hi = keys[key] == ButtonEventType::BUTTON_EVENT_DOWN ? 0x4000 : 0;
		lparam = MAKELPARAM(1, command.val2 | hi);
		keys[key] = ButtonEventType::BUTTON_EVENT_DOWN;
	}
	else {
		//command.event1 == (int16_t)ButtonEventType::BUTTON_EVENT_UP;
		message = DV_KEYUP;
		lparam = MAKELPARAM(
			1,
			command.val2 | // scancode
			0x4000 | 0x8000
		);
		keys[key] = ButtonEventType::BUTTON_EVENT_UP;
	}

	commands[lastCommand % 128] = command;
	_commandQueue.push(command);
	ORIGINAL_WND_PROC_MULTIPLE(wnd, windowsCount, message, key, lparam);
	ORIGINAL_WND_PROC_MULTIPLE(wnd, windowsCount, DV_INPUT, RIM_INPUT, MAKELPARAM(lastCommand % 128, 0));
	lastCommand++;
}

int KeyboardShadowController::ReadInput(InputCommand commandBuffer[], size_t size)
{
	int read = 0;
	while (size > 0 && _commandQueue.size() > 0) {
		auto comm = _commandQueue.front();
		_commandQueue.pop();
		memcpy(&commandBuffer[read], &comm, sizeof(InputCommand));
		read++;
		size--;
	}
	return read;
}
  
InputCommand KeyboardShadowController::GetInputCommand(HRAWINPUT hRawInput)
{
	return commands[(int)hRawInput % 128];
}