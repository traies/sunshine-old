#pragma once
#include "stdafx.h"
#include <stdint.h>
#include <Windows.h>

#define DV_INPUT		(WM_APP + 200)
#define DV_MOUSEMOVE	(DV_INPUT + 1)
#define DV_KEYUP		(DV_INPUT + 2)
#define DV_KEYDOWN		(DV_INPUT + 3)
#define DV_LBUTTONDOWN	(DV_INPUT + 4)
#define DV_LBUTTONUP	(DV_INPUT + 5)
#define DV_RBUTTONDOWN	(DV_INPUT + 6)
#define DV_RBUTTONUP	(DV_INPUT + 7)
#define DV_MBUTTONDOWN	(DV_INPUT + 8)
#define DV_MBUTTONUP	(DV_INPUT + 9)
#define DV_XBUTTONDOWN	(DV_INPUT + 10)
#define DV_XBUTTONUP	(DV_INPUT + 11)
#define DV_MOUSEWHEEL	(DV_INPUT + 12)

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
