#pragma once
#include "UDPServer.h"
#include "WndProcHook.h"

class InputPipeline
{
public:
	InputPipeline() : server(1235), wnd(GetWindowForThisProc()), wndProcHook(wnd)
	{
	}
	~InputPipeline() {};
	void Run();
private:
	static HWND validHwnd;
	HWND wnd;
	HWND GetWindowForThisProc();
	UDPServer server;
	WndProcHook wndProcHook;
	static BOOL CALLBACK GetWindowCallback(HWND wnd, LPARAM currProc);
	
};

