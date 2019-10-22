#include "stdafx.h"
#include "InputCommand.h"

void MakeMouseCommand(InputCommand& command, int16_t x, int16_t y, MouseButton mouseButton, ButtonEventType mouseEvent)
{
	command.type = (int16_t)InputCommandType::INPUT_COMMAND_MOUSE;
	command.val1 = x;
	command.val2 = y;
	command.event1 = (int16_t)mouseButton;
	command.event2 = (int16_t)mouseEvent;
}

void MakeKeyboardCommand(InputCommand& command, int16_t key, int16_t scancode, ButtonEventType ev)
{
	command.type = (int16_t)InputCommandType::INPUT_COMMAND_KEYBOARD;
	command.val1 = key;
	command.val2 = scancode;
	command.event1 = (int16_t)ev;
}
