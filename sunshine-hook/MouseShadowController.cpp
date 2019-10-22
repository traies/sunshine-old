#include "stdafx.h"
#include "MouseShadowController.h"
#include "WndProcHook.h"

MouseShadowController* MouseShadowController::instance = nullptr;

void UpdateMouseButton(int16_t ev, bool& mouseButton)
{
	if (ev == (int16_t)ButtonEventType::BUTTON_EVENT_DOWN) {
		mouseButton = true;
	}
	else if (ev == (int16_t)ButtonEventType::BUTTON_EVENT_UP) {
		mouseButton = false;
	}
}

void MouseShadowController::UpdateState(InputCommand command, HWND wnd)
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
			message = WM_LBUTTONDOWN;
		}
		else if (command.event2 == (int16_t)ButtonEventType::BUTTON_EVENT_UP) {
			message = WM_LBUTTONUP;
		}
		break;
	case (int16_t)MouseButton::MOUSE_BUTTON_RIGHT:
		if (command.event2 == (int16_t)ButtonEventType::BUTTON_EVENT_DOWN) {
			message = WM_RBUTTONDOWN;
		}
		else if (command.event2 == (int16_t)ButtonEventType::BUTTON_EVENT_UP) {
			message = WM_RBUTTONUP;
		}
		break;
	case (int16_t)MouseButton::MOUSE_BUTTON_MIDDLE:
		if (command.event2 == (int16_t)ButtonEventType::BUTTON_EVENT_DOWN) {
			message = WM_MBUTTONDOWN;
		}
		else if (command.event2 == (int16_t)ButtonEventType::BUTTON_EVENT_UP) {
			message = WM_MBUTTONUP;
		}
		break;
	case (int16_t)MouseButton::MOUSE_BUTTON_4:
		if (command.event2 == (int16_t)ButtonEventType::BUTTON_EVENT_DOWN) {
			message = WM_XBUTTONDOWN;
		}
		else if (command.event2 == (int16_t)ButtonEventType::BUTTON_EVENT_UP) {
			message = WM_XBUTTONUP;
		}
		mouseBtn = MAKEWPARAM(mouseBtn, XBUTTON1);
		break;
	case (int16_t)MouseButton::MOUSE_BUTTON_5:
		if (command.event2 == (int16_t)ButtonEventType::BUTTON_EVENT_DOWN) {
			message = WM_XBUTTONDOWN;
		}
		else if (command.event2 == (int16_t)ButtonEventType::BUTTON_EVENT_UP) {
			message = WM_XBUTTONUP;
		}
		mouseBtn = MAKEWPARAM(mouseBtn, XBUTTON2);
		break;
	case (int16_t)MouseButton::MOUSE_SCROLL:
		message = WM_MOUSEWHEEL;
		mouseBtn = MAKEWPARAM(mouseBtn, command.event2);
		scrollOffset = command.event2;
	}

	if (message > 0) {
		ORIGINAL_WND_PROC(wnd, message, wparam, MAKELPARAM(command.val1, command.val2));
	}
	// Always send MOUSEMOVE for consistency
	ORIGINAL_WND_PROC(wnd, WM_MOUSEMOVE, wparam, MAKELPARAM(command.val1, command.val2));
	lastX = command.val1;
	lastY = command.val2;
	
	// Raw Input 
	ORIGINAL_WND_PROC(wnd, WM_INPUT, RIM_INPUT, 1); // RAW INPUT HANDLE

}

