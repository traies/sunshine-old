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
		fhook = std::make_unique<FocusHook>();
		auto b = fhook->Install();
		wndProcHook = std::make_unique<WndProcHook>();
		wndProcHook->Install();

		std::thread updateWindow([&] {
			while (true) {
				windowsCount = GetWindowForThisProc(windows, 100);
				if (windowsCount > 0) {
					fhook->SetForegroundWindow(windows[0]);
					fhook->SetActiveWindow(windows[0]);
					fhook->SetFocusWindow(windows[0]);
					LOG(INFO) << "Changing window...";
					for (int i = 0; i < windowsCount; i++) {
						wndProcHook->Install(windows[i]);
					}
				}
				Sleep(10000);
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
	HWND windows[100];
	int windowsCount = 0;
	int GetWindowForThisProc(HWND* windows, size_t maxSize);
	UDPServer _server;
	static BOOL CALLBACK GetWindowCallback(HWND wnd, LPARAM currProc);
	int NextCommand(InputCommand& nextCommand);
};

