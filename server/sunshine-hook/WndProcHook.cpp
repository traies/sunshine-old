#include "stdafx.h"
#include "WndProcHook.h"

typedef DWORD (STDMETHODCALLTYPE* GETQUEUESTATUS)(
	UINT flags
);

static GETQUEUESTATUS getQueueStatus = nullptr;

static DWORD HookGetQueueStatus(UINT flags)
{
	LOG(INFO) << "GET QUEUE STATUS";
	auto res = getQueueStatus(flags);
	return res;
}

bool WndProcHook::Install() {
	HMODULE hMod = GetModuleHandle(TEXT("user32.dll"));

	getMessageW = (GETMESSAGE)GetProcAddress(hMod, "GetMessageW");
	if (getMessageW == nullptr) {
		LOG(ERROR) << "GetProcAddress failed for GetMessageW";
		return false;
	}

	this->InstallHook("GetMessageW", getMessageW, HookGetMessageW);
	
	peekMessageW = (PEEKMESSAGE)GetProcAddress(hMod, "PeekMessageW");
	if (peekMessageW == nullptr) {
		LOG(ERROR) << "GetProcAddress failed for PeekMessageW";
		return false;
	}
	this->InstallHook("PeekMessageW", peekMessageW, HookPeekMessageW);

	getMessageA = (GETMESSAGE)GetProcAddress(hMod, "GetMessageA");
	if (getMessageA == nullptr) {
		LOG(ERROR) << "GetProcAddress failed for GetMessageA";
		return false;
	}
	this->InstallHook("GetMessageA", getMessageA, HookGetMessageA);

	peekMessageA = (PEEKMESSAGE)GetProcAddress(hMod, "PeekMessageA");
	if (peekMessageA == nullptr) {
		LOG(ERROR) << "GetProcAddress failed for PeekMessageA";
		return false;
	}
	this->InstallHook("PeekMessageA", peekMessageA, HookPeekMessageA);

	getQueueStatus = (GETQUEUESTATUS)GetProcAddress(hMod, "GetQueueStatus");
	if (getQueueStatus == nullptr) {
		LOG(ERROR) << "GetProcAddress failed for GetQueueStatus";
		return false;
	}
	this->InstallHook("GetQueueStatus", getQueueStatus, HookGetQueueStatus);
	return true;
}

std::map<HWND, WNDPROC>* ORIGINAL_WND_PROC_MAP = new std::map<HWND, WNDPROC>();
LRESULT ORIGINAL_WND_PROC(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if ((*ORIGINAL_WND_PROC_MAP)[hWnd]) {
		return CallWindowProc((*ORIGINAL_WND_PROC_MAP)[hWnd], hWnd, message, wParam, lParam);
	}
	else {
		LOG(ERROR) << "No original WND PROC for this HWND";
		return 0;
	}
}

void ORIGINAL_WND_PROC_MULTIPLE(HWND * windows, int count, UINT message, WPARAM wParam, LPARAM lParam)
{
	BOOL res = false;
	for (int i = 0; i < count; i++) {
		res = PostMessageA(windows[i], message, wParam, lParam);
		if (!res) {
			LOG(INFO) << "PostMessage failed. Window: " << windows[i] << GetLastError();;
		}
	/*	auto thread = GetWindowThreadProcessId(windows[i], nullptr);
		res = PostThreadMessage(thread, message, wParam, lParam);
		if (!res) {
			LOG(INFO) << "PostThreadMessage failed. Thread: " << thread << GetLastError();;
		}*/
	}
	/*res = PostMessage(HWND_BROADCAST, message, wParam, lParam);
	if (!res) {
		LOG(INFO) << "PostMessageBroadcast failed " << GetLastError();
	}*/
}

void ADD_WND_PROC(HWND hWnd, WNDPROC proc)
{
	(*ORIGINAL_WND_PROC_MAP)[hWnd] = proc;
}