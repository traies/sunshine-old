#include "stdafx.h"
#include "Hook.h"
#include "..\easyloggingpp\easylogging++.h"
#include <easyhook.h>

bool Hook::InstallHook(std::string name, void * oldfunc, void *newfunc) 
{
	ULONG threadIds[] = { 0 };
	HOOK_TRACE_INFO hookTraceInfo;
	ZeroMemory(&hookTraceInfo, sizeof(hookTraceInfo));
	//	Remember to save hookTraceInfo, it is important for release
	if (LhInstallHook(oldfunc, newfunc, NULL, &hookTraceInfo) != 0) {
		LOG(ERROR) << "Hook for function " << name << " failed.";
		return false;
	}

	if (LhSetExclusiveACL(threadIds, 1, &hookTraceInfo)) {
		LOG(ERROR) << "Cannot activate hook for function " << name;
		return false;
	}

	LOG(INFO) << "Hook " << name << " name.";
	return true;
}

//	Useless window procedure, used for Hook installation
LRESULT CALLBACK TempWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}