#pragma once
#include "Hook.h"
#include <Windows.h>
#include "..\easyloggingpp\easylogging++.h"
#include <thread>
#include <map>
#include "InputCommand.h"

#define MAX_REHOOKS			20
#define REHOOK_TIMEOUT		10000

typedef BOOL(STDMETHODCALLTYPE* GETMESSAGE)(
	LPMSG lpMsg,
	HWND hWnd,
	UINT wMsgFilterMin,
	UINT wMsgFilterMax
	);

typedef BOOL(STDMETHODCALLTYPE* PEEKMESSAGE)(
	LPMSG lpMsg,
	HWND hWnd,
	UINT wMsgFilterMin,
	UINT wMsgFilterMax,
	UINT wRemoteMsg
	);


/*	The idea of this hook is to prevent messages informing the
	the game that it has lost focus. This allows us to keep sending 
	inputs to the game even if minimized.
*/
LRESULT ORIGINAL_WND_PROC(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void ORIGINAL_WND_PROC_MULTIPLE(HWND* windows, int count, UINT message, WPARAM wParam, LPARAM lParam);
void ADD_WND_PROC(HWND hWnd, WNDPROC);

static GETMESSAGE getMessageW = nullptr;
static GETMESSAGE getMessageA = nullptr;
static PEEKMESSAGE peekMessageW = nullptr;
static PEEKMESSAGE peekMessageA = nullptr;

inline static BOOL TranslateJustDummyValue(UINT& message)
{
	switch (message) {
	case DV_MOUSEMOVE:
		message = WM_MOUSEMOVE;
		break;
	case DV_INPUT:
		message = WM_INPUT;
		break;
	case DV_KEYUP:
		message = WM_KEYUP;
		break;
	case DV_KEYDOWN:
		message = WM_KEYDOWN;
		break;
	case DV_LBUTTONDOWN:
		message = WM_LBUTTONDOWN;
		break;
	case DV_LBUTTONUP:
		message = WM_LBUTTONUP;
		break;
	case DV_RBUTTONDOWN:
		message = WM_RBUTTONDOWN;
		break;
	case DV_RBUTTONUP:
		message = WM_RBUTTONUP;
		break;
	case DV_MBUTTONDOWN:
		message = WM_MBUTTONDOWN;
		break;
	case DV_MBUTTONUP:
		message = WM_MBUTTONUP;
		break;
	case DV_XBUTTONDOWN:
		message = WM_XBUTTONDOWN;
		break;
	case DV_XBUTTONUP:
		message = WM_XBUTTONUP;
		break;
	case DV_MOUSEWHEEL:
		message = WM_MOUSEWHEEL;
		break;
	}
	return true;
}

inline static BOOL TranslateDummyValue(UINT& message, WPARAM wparam)
{
	switch (message) {
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MBUTTONDBLCLK:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_XBUTTONDBLCLK:
	case WM_MOUSEMOVE:
	case WM_MOUSEHOVER:
	case WM_MOUSELEAVE:
	case WM_MOUSEWHEEL:
	case WM_NCMOUSEMOVE:
	case WM_SETCURSOR:
	case WM_NCHITTEST:
	case WM_INPUT_DEVICE_CHANGE:
	case WM_CAPTURECHANGED:
	//case WM_CHAR:
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_INPUT:
	case WM_COMMAND:
	case WM_APPCOMMAND:
		message = WM_NULL;
		return false;
	case WM_ACTIVATE:
		if (wparam == WA_ACTIVE || wparam == WA_CLICKACTIVE) {
			LOG(INFO) << "Window activated.";
			return true;
		}
		else {
			LOG(INFO) << "Window inactivated  55.";
			//bool r = ShowWindow(hWnd, SW_HIDE);
			message = WM_NULL;
			return false;
		}
		break;
	case WM_KILLFOCUS:
		message = WM_NULL;
		return false;
	case WM_SETFOCUS:
		return true;
	}
	return TranslateJustDummyValue(message);
}

static BOOL commonGetMessage(
	LPMSG lpMsg,
	HWND hwnd,
	UINT wMsgFilterMin,
	UINT wMsgFilterMax,
	GETMESSAGE func
)
{
	BOOL res = func(lpMsg, hwnd, wMsgFilterMin, wMsgFilterMax);
	if (res) {
		res = TranslateDummyValue(lpMsg->message, lpMsg->wParam);
	}
	return res;
}

static BOOL commonPeekMessage(
	LPMSG lpMsg,
	HWND hwnd,
	UINT wMsgFilterMin,
	UINT wMsgFilterMax,
	UINT wRemoveMsg,
	PEEKMESSAGE func
)
{
	BOOL res = func(lpMsg, hwnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
	if (res) {
		res = TranslateDummyValue(lpMsg->message, lpMsg->wParam);
	}
	return res;
}


static BOOL HookGetMessageW(
	LPMSG lpMsg,
	HWND  hWnd,
	UINT  wMsgFilterMin,
	UINT  wMsgFilterMax
)
{
	return commonGetMessage(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, getMessageW);
}

static BOOL HookGetMessageA(
	LPMSG lpMsg,
	HWND  hWnd,
	UINT  wMsgFilterMin,
	UINT  wMsgFilterMax
)
{
	return commonGetMessage(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, getMessageA);
}

static BOOL HookPeekMessageW(
	LPMSG lpMsg,
	HWND  hWnd,
	UINT  wMsgFilterMin,
	UINT  wMsgFilterMax,
	UINT  wRemoveMsg
)
{
	return commonPeekMessage(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg, peekMessageW);
}

static BOOL HookPeekMessageA(
	LPMSG lpMsg,
	HWND  hWnd,
	UINT  wMsgFilterMin,
	UINT  wMsgFilterMax,
	UINT wRemoveMsg
)
{
	return commonPeekMessage(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg, peekMessageA);
}

class WndProcHook: public Hook {

private:
	HWND window;

	static LRESULT CALLBACK TempWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		switch (message) {
		case WM_ACTIVATE:
			if (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE) {
				LOG(INFO) << "Window activated.";
				return ORIGINAL_WND_PROC(hWnd, message, wParam, lParam);
			}
			else {
				LOG(INFO) << "Window inactivated  56.";
				return 0;// DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		}
		TranslateJustDummyValue(message);
		return ORIGINAL_WND_PROC(hWnd, message, wParam, lParam);
	}
public:
	bool Install() override;
	bool Uninstall() override
	{
		//TODO correct
		return true;
	}
	bool Install(HWND window)
	{
		auto temp = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)&TempWndProc);
		if (temp == 0) {
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
			LOG(ERROR) << "Could not SetWindowLongPtr. ";// << (wchar_t*)lpMsgBuf;
			return false;
		}
		else if (temp != &TempWndProc) {
			ADD_WND_PROC(window, temp);
			LOG(INFO) << "Old Proc " << temp;
			//ORIGINAL_WND_PROC(window, WM_ACTIVATE, WA_ACTIVE, 0);
			//ShowWindow(window, SW_HIDE);
			return false;
		}
		LOG(INFO) << "SetWindowLongPtr succeded.";
		return true;
	};
	~WndProcHook() {};
};