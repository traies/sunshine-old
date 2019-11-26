#include "stdafx.h"
#include "MouseShadowController.h"
#include "WndProcHook.h"

MouseShadowController* MouseShadowController::instance = nullptr;

static void UpdateMouseButton(int16_t ev, bool& mouseButton)
{
	if (ev == (int16_t)ButtonEventType::BUTTON_EVENT_DOWN) {
		mouseButton = true;
	}
	else if (ev == (int16_t)ButtonEventType::BUTTON_EVENT_UP) {
		mouseButton = false;
	}
}

inline static void UpdateRawMouseButton(int16_t ev, USHORT &buttonFlags, USHORT downFlag, USHORT upFlag)
{
	if (ev == (int16_t)ButtonEventType::BUTTON_EVENT_DOWN) {
		buttonFlags |= downFlag;
	}
	else if (ev == (int16_t)ButtonEventType::BUTTON_EVENT_UP) {
		buttonFlags |= upFlag;
	}
}

inline static USHORT ButtonFlags(InputCommand command)
{
	USHORT buttonFlags = 0;
	if (command.event1 == (int16_t)MouseButton::MOUSE_BUTTON_LEFT) {
		UpdateRawMouseButton(command.event2, buttonFlags, RI_MOUSE_LEFT_BUTTON_DOWN, RI_MOUSE_LEFT_BUTTON_UP);
	}
	else if (command.event1 == (int16_t)MouseButton::MOUSE_BUTTON_RIGHT) {
		UpdateRawMouseButton(command.event2, buttonFlags, RI_MOUSE_RIGHT_BUTTON_DOWN, RI_MOUSE_RIGHT_BUTTON_UP);
	}
	else if (command.event1 == (int16_t)MouseButton::MOUSE_BUTTON_MIDDLE) {
		UpdateRawMouseButton(command.event2, buttonFlags, RI_MOUSE_MIDDLE_BUTTON_DOWN, RI_MOUSE_MIDDLE_BUTTON_UP);
	}
	else if (command.event1 == (int16_t)MouseButton::MOUSE_BUTTON_4) {
		UpdateRawMouseButton(command.event2, buttonFlags, RI_MOUSE_BUTTON_4_DOWN, RI_MOUSE_BUTTON_4_UP);
	}
	else if (command.event1 == (int16_t)MouseButton::MOUSE_BUTTON_5) {
		UpdateRawMouseButton(command.event2, buttonFlags, RI_MOUSE_BUTTON_5_DOWN, RI_MOUSE_BUTTON_5_UP);
	}
	else if (command.event1 == (int16_t)MouseButton::MOUSE_SCROLL) {
		buttonFlags |= RI_MOUSE_WHEEL;
	}
	return buttonFlags;
}

void MouseShadowController::UpdateState(InputCommand command, HWND * wnd, int windowsCount)
{
	//  UPDATE internal buttons state;
	if (command.event1 == (int16_t)MouseButton::MOUSE_BUTTON_LEFT) {
		UpdateMouseButton(command.event2, lb);
	}
	else if (command.event1 == (int16_t)MouseButton::MOUSE_BUTTON_RIGHT) {
		UpdateMouseButton(command.event2, rb);
	}
	else if (command.event1 == (int16_t) MouseButton::MOUSE_BUTTON_MIDDLE) {
		UpdateMouseButton(command.event2, mb);
	}
	else if (command.event1 == (int16_t)MouseButton::MOUSE_BUTTON_4) {
		UpdateMouseButton(command.event2, x1b);
	}
	else if (command.event1 == (int16_t)MouseButton::MOUSE_BUTTON_5) {
		UpdateMouseButton(command.event2, x2b);
	}

	// Parse WPARAM modifiers
	WPARAM mods = 0; // KeyModifiers();
	WPARAM mouseBtn = 0;
	if (lb) {
		mouseBtn |= MK_LBUTTON;
	}
	if (rb) {
		mouseBtn |= MK_RBUTTON;
	}
	if (mb) {
		mouseBtn |= MK_MBUTTON;
	}
	if (x1b) {
		mouseBtn |= MK_XBUTTON1;
	}
	if (x2b) {
		mouseBtn |= MK_XBUTTON2;
	}

	WPARAM wparam = mods | mouseBtn;
	UINT message = 0;
	// Send MOUSE event to window.
	switch (command.event1) {
	case (int16_t)MouseButton::MOUSE_BUTTON_LEFT:
		if (command.event2 == (int16_t)ButtonEventType::BUTTON_EVENT_DOWN) {
			message = DV_LBUTTONDOWN;
			this->keys[VK_LBUTTON] = ButtonEventType::BUTTON_EVENT_DOWN;
		}
		else if (command.event2 == (int16_t)ButtonEventType::BUTTON_EVENT_UP) {
			message = DV_LBUTTONUP;
			this->keys[VK_LBUTTON] = ButtonEventType::BUTTON_EVENT_UP;
		}
		break;
	case (int16_t)MouseButton::MOUSE_BUTTON_RIGHT:
		if (command.event2 == (int16_t)ButtonEventType::BUTTON_EVENT_DOWN) {
			message = DV_RBUTTONDOWN;
			this->keys[VK_RBUTTON] = ButtonEventType::BUTTON_EVENT_DOWN;
		}
		else if (command.event2 == (int16_t)ButtonEventType::BUTTON_EVENT_UP) {
			message = DV_RBUTTONUP;
			this->keys[VK_RBUTTON] = ButtonEventType::BUTTON_EVENT_UP;
		}
		break;
	case (int16_t)MouseButton::MOUSE_BUTTON_MIDDLE:
		if (command.event2 == (int16_t)ButtonEventType::BUTTON_EVENT_DOWN) {
			message = DV_MBUTTONDOWN;
			this->keys[VK_MBUTTON] = ButtonEventType::BUTTON_EVENT_DOWN;
		}
		else if (command.event2 == (int16_t)ButtonEventType::BUTTON_EVENT_UP) {
			message = DV_MBUTTONUP;
			this->keys[VK_MBUTTON] = ButtonEventType::BUTTON_EVENT_UP;
		}
		break;
	case (int16_t)MouseButton::MOUSE_BUTTON_4:
		if (command.event2 == (int16_t)ButtonEventType::BUTTON_EVENT_DOWN) {
			message = DV_XBUTTONDOWN;
			this->keys[VK_XBUTTON1] = ButtonEventType::BUTTON_EVENT_DOWN;
		}
		else if (command.event2 == (int16_t)ButtonEventType::BUTTON_EVENT_UP) {
			message = DV_XBUTTONUP;
			this->keys[VK_XBUTTON1] = ButtonEventType::BUTTON_EVENT_UP;
		}
		mouseBtn = MAKEWPARAM(mouseBtn, XBUTTON1);
		break;
	case (int16_t)MouseButton::MOUSE_BUTTON_5:
		if (command.event2 == (int16_t)ButtonEventType::BUTTON_EVENT_DOWN) {
			message = DV_XBUTTONDOWN;
			this->keys[VK_XBUTTON2] = ButtonEventType::BUTTON_EVENT_DOWN;
		}
		else if (command.event2 == (int16_t)ButtonEventType::BUTTON_EVENT_UP) {
			message = DV_XBUTTONUP;
			this->keys[VK_XBUTTON2] = ButtonEventType::BUTTON_EVENT_UP;
		}
		mouseBtn = MAKEWPARAM(mouseBtn, XBUTTON2);
		break;
	case (int16_t)MouseButton::MOUSE_SCROLL:
		message = DV_MOUSEWHEEL;
		mouseBtn = MAKEWPARAM(mouseBtn, command.event2);
		scrollOffset = command.event2;
	}

	long lastXRelative = command.val1 - lastX;
	long lastYRelative = command.val2 - lastY;
	lastX = command.val1;
	lastY = command.val2;

	int idx = lastCommand % 128;
	commands[idx].lLastX = lastXRelative;
	commands[idx].lLastY = lastYRelative;
	commands[idx].usButtonFlags = ButtonFlags(command);
	commands[idx].usButtonData = message == DV_MOUSEWHEEL ? scrollOffset: 0;
	commands[idx].usFlags = MOUSE_MOVE_RELATIVE;

	if (message > 0) {
		ORIGINAL_WND_PROC_MULTIPLE(wnd, windowsCount, message, wparam, MAKELPARAM(command.val1, command.val2));
	}
	else {
		// Always send MOUSEMOVE for consistency
		ORIGINAL_WND_PROC_MULTIPLE(wnd, windowsCount, DV_MOUSEMOVE, wparam, MAKELPARAM(command.val1, command.val2));
	}
	
	// Raw Input 
	ORIGINAL_WND_PROC_MULTIPLE(wnd, windowsCount, DV_INPUT, RIM_INPUT, MAKELPARAM(idx + 128, 0)); // RAW INPUT HANDLE

	lastCommand++;
}

RAWMOUSE MouseShadowController::GetRawMouse(HRAWINPUT hRaw)
{
	return commands[(int)hRaw - 128];
}

