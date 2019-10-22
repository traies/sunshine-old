#pragma once
#include "UDPServer.h"
#include "WndProcHook.h"
#include "FocusHook.h"
#include "ControlHook.h"
#include "InputCommand.h"

class InputPipeline
{
public:
	InputPipeline() : _server("1235")
	{
		wnd = GetWindowForThisProc();
		fhook = std::make_unique<FocusHook>();
		fhook->SetWindow(wnd);
		auto b = fhook->Install();
		
		wndProcHook = std::make_unique<WndProcHook>();
		wndProcHook->Install(wnd);

		std::thread updateWindow([&] {
			while (true) {
				auto newWnd = GetWindowForThisProc();
				if (newWnd != wnd) {
					LOG(INFO) << "Changing window...";
					wnd = newWnd;
					fhook->SetWindow(newWnd);
					wndProcHook->Install(newWnd);
				}
				Sleep(1000);
			}
		});

		updateWindow.detach();

		ControlHook chook;
		chook.Install();
	}
	~InputPipeline() 
	{
	};
	void Run();
private:
	static HWND validHwnd[100];
	static int validHwndCount;
	std::unique_ptr<FocusHook> fhook;
	std::unique_ptr<WndProcHook> wndProcHook;
	HWND wnd;
	HWND GetWindowForThisProc();
	UDPServer _server;
	static BOOL CALLBACK GetWindowCallback(HWND wnd, LPARAM currProc);
	int NextCommand(InputCommand& nextCommand);
};

