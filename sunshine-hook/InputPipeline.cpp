#include "stdafx.h"
#include "InputPipeline.h"
#include "..\easyloggingpp\easylogging++.h"
#include "WndProcHook.h"
#include <variant>
#define MAX_TRIES	100

void InputPipeline::Run()
{
	LOG(INFO) << "Run input pipeline";
	bool iResult = _server.Init();
	if (!iResult) {
		LOG(ERROR) << "Input server initialization failed.";
		return;
	}
	InputCommand command;
	bool leftButtonPressed = false;
	while (true) {
		ZeroMemory(&command, sizeof(command));
		int read = NextCommand(command);
		if (read <= 0) {
			LOG(ERROR) << "Input socket was closed.";
			break;
		}
		if (command.type == (int16_t) InputCommandType::INPUT_COMMAND_MOUSE) {
			// Update Mouse state in MouseShadowManager
			if (command.event1 == (int16_t)ButtonEventType::BUTTON_EVENT_DOWN) {
				ORIGINAL_WND_PROC(wnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(command.val1, command.val2));
				leftButtonPressed = true;
				//ORIGINAL_WND_PROC(wnd, WM_MBUTTONDOWN, MK_LBUTTON, MAKELPARAM(command.val1, command.val2));
			} else if (command.event1 == (int16_t)ButtonEventType::BUTTON_EVENT_UP) {
				ORIGINAL_WND_PROC(wnd, WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(command.val1, command.val2));
				leftButtonPressed = false;
				//ORIGINAL_WND_PROC(wnd, WM_MBUTTONUP, MK_LBUTTON, MAKELPARAM(command.val1, command.val2));
			} else if (command.event2 == (int16_t)ButtonEventType::BUTTON_EVENT_DOWN) {
				ORIGINAL_WND_PROC(wnd, WM_RBUTTONDOWN, MK_RBUTTON, MAKELPARAM(command.val1, command.val2));
				//ORIGINAL_WND_PROC(wnd, WM_MBUTTONDOWN, MK_RBUTTON, MAKELPARAM(command.val1, command.val2));
			} else if (command.event2 == (int16_t)ButtonEventType::BUTTON_EVENT_UP) {
				ORIGINAL_WND_PROC(wnd, WM_RBUTTONUP, MK_RBUTTON, MAKELPARAM(command.val1, command.val2));
				//ORIGINAL_WND_PROC(wnd, WM_MBUTTONUP, MK_RBUTTON, MAKELPARAM(command.val1, command.val2));
			}

			if (leftButtonPressed) {
				ORIGINAL_WND_PROC(wnd, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(command.val1, command.val2));
			}
			else {
				ORIGINAL_WND_PROC(wnd, WM_MOUSEMOVE, 0, MAKELPARAM(command.val1, command.val2));
			}
		}
		else if (command.type == (int16_t) InputCommandType::INPUT_COMMAND_KEYBOARD) {
			// Update Keyboard state in KeyboardShadowManager

		}
		//if (ORIGINAL_WND_PROC != nullptr) {
		//	ORIGINAL_WND_PROC(wnd, WM_MOUSEMOVE, 0, MAKELPARAM(0, 0));
		//	Sleep(1000);
		//	ORIGINAL_WND_PROC(wnd, WM_MOUSEMOVE, 0, MAKELPARAM(1000, 1000));
		//}
		//else {
		//	Sleep(1000);
		//}
	}
}

HWND InputPipeline::validHwnd[100];
int InputPipeline::validHwndCount;
HWND InputPipeline::GetWindowForThisProc()
{
	for (int i = 0; i < MAX_TRIES; i++) {
		DWORD currProc = GetCurrentProcessId();
		EnumWindows(&InputPipeline::GetWindowCallback, (LPARAM)&currProc);
		if (validHwndCount == 0) {
			LOG(INFO) << "Could not find the process window. Retrying..";
		}
		else {
			break;
		}
		Sleep(1000);
		LOG(INFO) << "Retrying window find..";
	}
	if (validHwndCount == 0) {
		LOG(ERROR) << "Hooking input failed.";
		throw std::runtime_error("Could not find a valid window");
	}
	LOG(INFO) << "Valid Windows were " << validHwndCount;

	for (int i = 0; i < validHwndCount; i++) {
		if (IsWindowVisible(validHwnd[i])) {
			LOG(INFO) << "Last Valid input " << validHwnd[i];
			return validHwnd[i];
		}
	}
	LOG(ERROR) << "Hooking input failed.";
	throw std::runtime_error("Could not find a valid window");
}

BOOL CALLBACK InputPipeline::GetWindowCallback(HWND wnd, LPARAM currProc)
{
	DWORD proc = 0;
	GetWindowThreadProcessId(wnd, &proc);
	if (*(LPDWORD)currProc == proc) {
		validHwnd[validHwndCount++] = wnd;
		return true;
	}
	return true;
}

int InputPipeline::NextCommand(InputCommand& nextCommand)
{
	return _server.Receive(reinterpret_cast<char *>(&nextCommand), sizeof(InputCommand));
};