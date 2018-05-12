#include "stdafx.h"
#include "InputPipeline.h"
#include "..\easyloggingpp\easylogging++.h"
#define MAX_TRIES	10

void InputPipeline::Run()
{
	while (true) {
		UINT key = 0x41;
		PostMessage(wnd, WM_KEYDOWN, VK_RIGHT, 
			1 
			| MapVirtualKey(VK_RIGHT, MAPVK_VK_TO_VSC) << 16
			| 1 << 24);
		//PostMessage(wnd, WM_CHAR, key, MapVirtualKey(key, MAPVK_VK_TO_CHAR));
		//PostMessage(wnd, WM_KEYUP, key, MapVirtualKey(key, MAPVK_VK_TO_VSC));
		Sleep(1000);
	}
}

HWND InputPipeline::validHwnd;
HWND InputPipeline::GetWindowForThisProc()
{
	for (int i = 0; i < MAX_TRIES; i++) {
		DWORD currProc = GetCurrentProcessId();
		EnumWindows(&InputPipeline::GetWindowCallback, (LPARAM)&currProc);
		if (validHwnd == nullptr) {
			LOG(INFO) << "Could not find the process window. Retrying..";
		}
		else {
			break;
		}
		Sleep(1000);
	}
	if (validHwnd == nullptr) {
		LOG(ERROR) << "Hooking input failed.";
	}
	return validHwnd;
}

BOOL CALLBACK InputPipeline::GetWindowCallback(HWND wnd, LPARAM currProc)
{
	DWORD proc = 0;
	GetWindowThreadProcessId(wnd, &proc);
	if (*(LPDWORD)currProc == proc) {
		validHwnd = wnd;
		return false;
	}
	return true;
}
