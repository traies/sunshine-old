#pragma once
#include "Hook.h"
#include "../easyloggingpp/easylogging++.h"

typedef HWND (STDMETHODCALLTYPE * GETFOCUSTYPE)();
typedef HWND (STDMETHODCALLTYPE * SETFOCUSTYPE)(HWND wnd);
typedef bool (STDMETHODCALLTYPE * SETFOREGROUNDTYPE)(HWND wnd);
typedef bool (STDMETHODCALLTYPE * SETCURSORPOS)(int x, int y);
typedef bool (STDMETHODCALLTYPE * CLIPCURSOR)(RECT * rect);
static HWND activeWindow = nullptr;
static HWND foregroundWindow = nullptr;
static HWND focusWindow = nullptr;

static HWND WINAPI HookGetFocus()
{
	//LOG(INFO) << "calling get focus..." << focusWindow;
	return focusWindow;
}

static HWND WINAPI HookSetFocus(HWND wnd)
{
	//LOG(INFO) << "calling set focus...";
	focusWindow = wnd;
	return focusWindow;
}


static HWND WINAPI HookGetForegroundWindow()
{
	//LOG(INFO) << "calling get foreground window..." << foregroundWindow;
	return focusWindow;
}

static bool WINAPI HookSetForegroundWindow(HWND wnd)
{
	//LOG(INFO) << "calling set foreground window...";
	foregroundWindow = wnd;
	return true;
}

static HWND WINAPI HookGetActiveWindow()
{
	//LOG(INFO) << "calling get active window..." << activeWindow;
	return focusWindow;
}


static bool WINAPI HookSetActiveWindow(HWND wnd)
{
	//LOG(INFO) << "calling set active window...";
	activeWindow = wnd;
	return true;
}

static bool WINAPI HookSetCursorPos(int x, int y)
{
	//LOG(INFO) << "setting cursor pos: " << x <<", "<< y;
	return true;
}

static bool WINAPI HookClipCursor(RECT * rect)
{
	//LOG(INFO) << "setting clip cursor.";
	return true;
}

class FocusHook: public Hook
{
public:
	void SetForegroundWindow(HWND wnd) {
		foregroundWindow = wnd;
	}

	void SetActiveWindow(HWND wnd) {
		activeWindow = wnd;
	}

	void SetFocusWindow(HWND wnd) {
		focusWindow = wnd;
	}

	bool Install()
	{
		auto hMod = GetModuleHandle(TEXT("user32.dll"));
		if (hMod == nullptr) {
			hMod = LoadLibrary(TEXT("user32.dll"));
			if (hMod == nullptr) {
				LOG(ERROR) << "Load user32.dll module failed.";
				return false;
			}
		}

		auto getFocus = (GETFOCUSTYPE)GetProcAddress(hMod, "GetFocus");
		if (getFocus == nullptr) {
			LOG(ERROR) << "GetProcAddress for GetFocus failed";
			return false;
		}
		this->InstallHook("GetFocus", getFocus, HookGetFocus);

		auto setFocus = (GETFOCUSTYPE)GetProcAddress(hMod, "SetFocus");
		if (setFocus == nullptr) {
			LOG(ERROR) << "GetProcAddress for SetFocus failed";
			return false;
		}
		this->InstallHook("SetFocus", setFocus, HookSetFocus);

		auto getForegroundWindow = (GETFOCUSTYPE)GetProcAddress(hMod, "GetForegroundWindow");
		if (getForegroundWindow == nullptr) {
			LOG(ERROR) << "GetProcAddress for GetForegroundWindow failed";
			return false;
		}
		this->InstallHook("GetForegroundWindow", getForegroundWindow, HookGetForegroundWindow);


		auto setForegroundWindow = (GETFOCUSTYPE)GetProcAddress(hMod, "SetForegroundWindow");
		if (setForegroundWindow == nullptr) {
			LOG(ERROR) << "GetProcAddress for SetForegroundWindow failed";
			return false;
		}
		this->InstallHook("SetForegroundWindow", setForegroundWindow, HookSetForegroundWindow);

		auto getActiveWindow = (GETFOCUSTYPE)GetProcAddress(hMod, "GetActiveWindow");
		if (getActiveWindow == nullptr) {
			LOG(ERROR) << "GetProcAddress for GetActiveWindow failed";
			return false;
		}
		this->InstallHook("GetActiveWindow", getActiveWindow, HookGetActiveWindow);

		auto setActiveWindow = (GETFOCUSTYPE)GetProcAddress(hMod, "SetActiveWindow");
		if (setActiveWindow == nullptr) {
			LOG(ERROR) << "GetProcAddress for SetActiveWindow failed";
			return false;
		}
		this->InstallHook("SetForegroundWindow", setActiveWindow, HookSetActiveWindow);

		auto setCursorPos = (SETCURSORPOS)GetProcAddress(hMod, "SetCursorPos");
		if (setCursorPos == nullptr) {
			LOG(ERROR) << "GetProcAddress for SetCursorPos failed";
			return false;
		}
		this->InstallHook("SetCursorPos", setCursorPos, HookSetCursorPos);

		auto setPhysicalCursorPos = (SETCURSORPOS)GetProcAddress(hMod, "SetPhysicalCursorPos");
		if (setPhysicalCursorPos == nullptr) {
			LOG(ERROR) << "GetProcAddress for SetPhysical failed";
			return false;
		}
		this->InstallHook("SetPhysicalCursorPos", setPhysicalCursorPos, HookSetCursorPos);

		auto clipCursor = (CLIPCURSOR)GetProcAddress(hMod, "ClipCursor");
		if (clipCursor == nullptr) {
			LOG(ERROR) << "GetProcAddress for ClipCursor failed";
			return false;
		}
		this->InstallHook("ClipCursor", clipCursor, HookClipCursor);

		return true;
	};

	bool Uninstall() 
	{
		return false;
	};
};