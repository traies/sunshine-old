#include "stdafx.h"
#include "InputCommand.h"

void MakeMouseCommand(InputCommand& command, int16_t x, int16_t y, ButtonEventType mouse1, ButtonEventType mouse2)
{
	command.type = (int16_t) InputCommandType::INPUT_COMMAND_MOUSE;
	command.val1 = x;
	command.val2 = y;
	command.event1 = (int16_t)mouse1;
	command.event2 = (int16_t)mouse2;
}

void MakeKeyboardCommand(InputCommand& command, int16_t key, ButtonEventType ev)
{
	command.type = (int16_t)InputCommandType::INPUT_COMMAND_KEYBOARD;
	command.val1 = key;
	command.event1 = (int16_t)ev;
}