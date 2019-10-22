#pragma once
#include <Windows.h>
#include "..\easyloggingpp\easylogging++.h"
#include <thread>
#include <map>

#define MAX_REHOOKS			20
#define REHOOK_TIMEOUT		100

/*	The idea of this hook is to prevent messages informing the
	the game that it has lost focus. This allows us to keep sending 
	inputs to the game even if minimized.
*/
LRESULT ORIGINAL_WND_PROC(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void ADD_WND_PROC(HWND hWnd, WNDPROC);
class WndProcHook {

private:
	HWND window;
	
	static LRESULT CALLBACK TempWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		switch (message) {
		case WM_ACTIVATE:
			if (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE) {
				LOG(INFO) << "Window activated.";
				return 0;
			}
			else {
				LOG(INFO) << "Window inactivated  55.";
				//bool r = ShowWindow(hWnd, SW_HIDE);
				return 0;
			}
			break;
		case WM_KILLFOCUS:
			LOG(INFO) << "Kill focus";
			return 0;
		case WM_SETFOCUS:
			LOG(INFO) << "Set focus";
			return 0;
		case WM_MOUSEMOVE:
		case WM_MOUSEHOVER:
		case WM_MOUSELEAVE:
		case WM_MOUSEWHEEL:
			return 0;
		case WM_INPUT:
		case WM_INPUT_DEVICE_CHANGE:
		case WM_CAPTURECHANGED:
			return 0;
		//case WM_CHAR:
		//case WM_KEYDOWN:
		//case WM_KEYUP:
		//	return 0;
		case WM_APPCOMMAND:
			return 0;
		}
		//return 0;
		return ORIGINAL_WND_PROC(hWnd, message, wParam, lParam);
	}
public:
	void InstallHook();

	void Install(HWND window)
	{
		LOG(INFO) << "WND HOOK";
		for (int i=0; i < MAX_REHOOKS; i++) {
			auto temp = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)&TempWndProc);
			if (temp == nullptr) {
				DWORD dw = GetLastError();
				LPVOID lpMsgBuf;
				FormatMessage(
					FORMAT_MESSAGE_ALLOCATE_BUFFER |
					FORMAT_MESSAGE_FROM_SYSTEM |
					FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					dw,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPTSTR)&lpMsgBuf,
					0, NULL);
				LOG(ERROR) << "Could not SetWindowLongPtr. " << lpMsgBuf;
			}
			else if (temp != &TempWndProc) {
				ADD_WND_PROC(window, temp);
				LOG(INFO) << "Old Proc " << temp;
				ORIGINAL_WND_PROC(window, WM_ACTIVATE, WA_ACTIVE, 0);
				//ShowWindow(window, SW_HIDE);
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