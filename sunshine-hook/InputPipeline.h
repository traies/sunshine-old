#pragma once
#include "UDPServer.h"
#include "WndProcHook.h"
#include "FocusHook.h"
#include "ControlHook.h"
#include "InputCommand.h"

class InputPipeline
{
public:
	InputPipeline() : _server("1235"), wnd(GetWindowForThisProc()), wndProcHook(wnd)
	{
		FocusHook fhook(wnd);
		auto b = fhook.Install();

		ControlHook chook;
		chook.Install();
	}
	~InputPipeline() {};
	void Run();
private:
	static HWND validHwnd[100];
	static int validHwndCount;
	HWND wnd;
	HWND GetWindowForThisProc();
	UDPServer _server;
	WndProcHook wndProcHook;
	static BOOL CALLBACK GetWindowCallback(HWND wnd, LPARAM currProc);
	int NextCommand(InputCommand& nextCommand);
};

