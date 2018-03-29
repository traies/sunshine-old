#pragma once
#include "UDPServer.h"

class InputPipeline
{
public:
	InputPipeline() : server(1235)
	{
		wnd = GetWindowForThisProc();
	}
	~InputPipeline() {};
private:
	static HWND validHwnd;
	HWND wnd;
	HWND GetWindowForThisProc();
	UDPServer server;
	static BOOL CALLBACK GetWindowCallback(HWND wnd, LPARAM currProc);
};

