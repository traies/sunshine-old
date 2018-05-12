#pragma once
#include <Windows.h>
#include "..\easyloggingpp\easylogging++.h"
#include <thread>

/*	The idea of this hook is to prevent messages informing the
	the game that it has lost focus. This allows us to keep sending 
	inputs to the game even if minimized.
*/
static WNDPROC oldProc;
class WndProcHook {

private:
	HWND window;
	

	
	static LRESULT CALLBACK TempWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		switch (message) {
		case WM_ACTIVATE:
			if (wParam == WA_ACTIVE) {
				LOG(INFO) << "Window activated.";
				break;
			}
			else {
				LOG(INFO) << "Window inactivated.";
				return 0;
			}
		case WM_KILLFOCUS:
			LOG(INFO) << "Killing focus";
			return 0;
		}
		return CallWindowProc(oldProc, hWnd, message, wParam, lParam);
	}
public:
	void InstallHook();
	WndProcHook(HWND window) : window(window)
	{
		SetWindowLongPtr(window, GWLP_USERDATA, (LONG_PTR)this);
		oldProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)&TempWndProc);
	};
	~WndProcHook() {};
};