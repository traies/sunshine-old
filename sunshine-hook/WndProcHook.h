#pragma once
#include <Windows.h>
#include "..\easyloggingpp\easylogging++.h"
#include <thread>

#define MAX_REHOOKS			20
#define REHOOK_TIMEOUT		100

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
			if (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE) {
				LOG(INFO) << "Window activated.";
				//break;
				//ShowWindow(hWnd, SW_RESTORE);
				break;
			}
			else {
				LOG(INFO) << "Window inactivated.";
				//bool r = ShowWindow(hWnd, SW_HIDE);
				return 0;
			}
			break;
		case WM_KILLFOCUS:
			return 0;
		case WM_SETFOCUS:
			break;
		case WM_MOUSEMOVE:
			//LOG(INFO) << "Mouse move";
			//return 0;
			break;
		}
		return CallWindowProc(oldProc, hWnd, message, wParam, lParam);
	}
public:
	void InstallHook();
	WndProcHook(HWND window) : window(window)
	{
		Sleep(REHOOK_TIMEOUT);
		ShowWindow(window, SW_HIDE);
		oldProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)&TempWndProc);
		Sleep(REHOOK_TIMEOUT);
		for (int i=0; i < MAX_REHOOKS; i++) {
			auto temp = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)&TempWndProc);
			if (temp != 0 && temp != &TempWndProc) {
				oldProc = temp;
				LOG(INFO) << "Changed oldProc " << GetLastError();
			}
			else if (temp == 0) {
				LOG(ERROR) << "Could not SetWindowLongPtr. " << GetLastError();
				break;
			}
			Sleep(REHOOK_TIMEOUT);
		};

		
	};
	~WndProcHook() {};
};