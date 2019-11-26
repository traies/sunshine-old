#pragma once
#include "UDPServer.h"
#include "WndProcHook.h"
#include "FocusHook.h"
#include "ControlHook.h"
#include "InputCommand.h"

class InputPipeline
{
public:
	InputPipeline(std::shared_ptr<UDPClientNew> endpoint);
	~InputPipeline() {};
	void Run();
private:
	static HWND validHwnd[100];
	static int validHwndCount;
	std::unique_ptr<FocusHook> fhook;
	std::unique_ptr<WndProcHook> wndProcHook;
	HWND windows[100] = {};
	int windowsCount = 0;
	int GetWindowForThisProc(HWND* windows, size_t maxSize);
	std::shared_ptr<UDPClientNew> _endpoint;
	static BOOL CALLBACK GetWindowCallback(HWND wnd, LPARAM currProc);
	int NextCommand(InputCommand& nextCommand);
};

