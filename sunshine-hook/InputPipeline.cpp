#include "stdafx.h"
#include "InputPipeline.h"
#include "..\easyloggingpp\easylogging++.h"
#include "WndProcHook.h"
#include <variant>
#include "MouseShadowController.h"
#include "KeyboardShadowController.h"

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
	ZeroMemory(&command, sizeof(command));
	while (true) {
		int read = NextCommand(command);
		if (read <= 0) {
			LOG(ERROR) << "Input socket was closed.";
			break;
		}
		if (command.type == (int16_t) InputCommandType::INPUT_COMMAND_MOUSE) {
			auto controller = MouseShadowController::GetInstance();
			controller->UpdateState(command, windows, windowsCount);
		}
		else if (command.type == (int16_t) InputCommandType::INPUT_COMMAND_KEYBOARD) {
			// Update Keyboard state in KeyboardShadowManager
			auto controller = KeyboardShadowController::GetInstance();
			controller->UpdateState(command, windows, windowsCount);
		}
	}
}

HWND InputPipeline::validHwnd[100];
int InputPipeline::validHwndCount;
int InputPipeline::GetWindowForThisProc(HWND * windows, size_t maxSize)
{
	validHwndCount = 0;
	for (int i = 0; i < MAX_TRIES; i++) {
		DWORD currProc = GetCurrentProcessId();
		EnumWindows(&InputPipeline::GetWindowCallback, (LPARAM)&currProc);
		if (validHwndCount == 0) {
			LOG(INFO) << "Could not find the process window. Retrying..";
		}
		else {
			int topLevelWnds = validHwndCount;
			for (int i = 0; i < topLevelWnds; i++) {
				EnumChildWindows(validHwnd[i], &InputPipeline::GetWindowCallback, (LPARAM)&currProc);
			}

			break;
		}
		Sleep(1000);
		LOG(INFO) << "Retrying window find..";
	}


	if (validHwndCount == 0) {
		LOG(ERROR) << "Hooking input failed.";
		throw std::runtime_error("Could not find a valid window");
	}
	//LOG(INFO) << "Valid Windows were " << validHwndCount;

	for (int i = 0; i < validHwndCount; i++) {
		windows[i] = validHwnd[i];
	}
	LOG(INFO) << "FOUND " << validHwndCount << " WINDOWS";
	return validHwndCount;
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