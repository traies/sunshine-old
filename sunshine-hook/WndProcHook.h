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
extern WNDPROC ORIGINAL_WND_PROC;
static bool STARTED;
class WndProcHook {

private:
	HWND window;
	
	static LRESULT CALLBACK TempWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		switch (message) {
		case WM_ACTIVATE:
			if (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE) {
				LOG(INFO) << "Window activated.";

				// Window should be activated only once
				if (!STARTED) {
					return 0;
				}
				STARTED = true;
				break;
			}
			else {
				LOG(INFO) << "Window inactivated  55.";
				//bool r = ShowWindow(hWnd, SW_HIDE);
				return 0;
			}
			break;
		case WM_KILLFOCUS:
			return 0;
		case WM_SETFOCUS:
			return 0;
		case WM_MOUSEMOVE:
		case WM_MOUSEHOVER:
		case WM_MOUSELEAVE:
		//case WM_NCMOUSEMOVE:
			return 0;
		case WM_INPUT:
			return 0;
		case WM_CHAR:
		case WM_KEYDOWN:
		case WM_KEYUP:
			return 0;
		//case WM_APPCOMMAND:
		//	return 0;
			//break;
		}
		return CallWindowProc(ORIGINAL_WND_PROC, hWnd, message, wParam, lParam);
	}
public:
	void InstallHook();
	WndProcHook(HWND window) : window(window)
	{
		Sleep(REHOOK_TIMEOUT);
		ShowWindow(window, SW_HIDE);
		ORIGINAL_WND_PROC = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)&TempWndProc);
		if (ORIGINAL_WND_PROC == nullptr) {
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
		LOG(INFO) << "Old Proc " << ORIGINAL_WND_PROC;
		Sleep(REHOOK_TIMEOUT);
		for (int i=0; i < MAX_REHOOKS; i++) {
			auto temp = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)&TempWndProc);
			if (temp != 0 && temp != &TempWndProc) {
				ORIGINAL_WND_PROC = temp;
				LOG(INFO) << "Changed oldProc ";
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