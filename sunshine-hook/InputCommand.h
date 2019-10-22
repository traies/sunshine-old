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

enum class MouseButton
{
	MOUSE_BUTTON_LEFT,
	MOUSE_BUTTON_RIGHT,
	MOUSE_BUTTON_MIDDLE,
	MOUSE_BUTTON_4,
	MOUSE_BUTTON_5,
	MOUSE_SCROLL,
};

struct InputCommand
{
	int16_t type;
	int16_t val1, val2;
	int16_t event1, event2;
};

void MakeMouseCommand(InputCommand& command, int16_t x, int16_t y, MouseButton mouseButton, ButtonEventType mouseEvent);
void MakeKeyboardCommand(InputCommand& command, int16_t key, int16_t scancode, ButtonEventType ev);
