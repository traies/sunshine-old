#pragma once
#include "UDPServer.h"
#include "WndProcHook.h"
#include "FocusHook.h"

class InputPipeline
{
public:
	InputPipeline() : server(1235), wnd(GetWindowForThisProc()), wndProcHook(wnd)
	{
		FocusHook fhook(wnd);
		auto b = fhook.Install();
	}
	~InputPipeline() {};
	void Run();
private:
	static HWND validHwnd[100];
	static int validHwndCount;
	HWND wnd;
	HWND GetWindowForThisProc();
	UDPServer server;
	WndProcHook wndProcHook;
	static BOOL CALLBACK GetWindowCallback(HWND wnd, LPARAM currProc);
	
};

