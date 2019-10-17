#pragma once
#include "stdafx.h"
#include <stdint.h>

enum class InputCommandType
{
	INPUT_COMMAND_MOUSE,
	INPUT_COMMAND_KEYBOARD,
};

enum class ButtonEventType
{
	BUTTON_EVENT_NONE,
	BUTTON_EVENT_UP,
	BUTTON_EVENT_DOWN,
};

struct InputCommand
{
	int16_t type;
	int16_t val1, val2;
	int16_t event1, event2;
};

InputCommand* MakeMouseCommand(int16_t x, int16_t y, ButtonEventType mouse1, ButtonEventType mouse2);
InputCommand* MakeKeyboardCommand(int16_t key, ButtonEventType ev);
