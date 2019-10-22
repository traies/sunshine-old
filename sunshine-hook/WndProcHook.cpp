#include "stdafx.h"
#include "WndProcHook.h"

void WndProcHook::InstallHook() {

}

std::map<HWND, WNDPROC>* ORIGINAL_WND_PROC_MAP = new std::map<HWND, WNDPROC>();
LRESULT ORIGINAL_WND_PROC(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if ((*ORIGINAL_WND_PROC_MAP)[hWnd]) {
		return (*ORIGINAL_WND_PROC_MAP)[hWnd](hWnd, message, wParam, lParam);
	}
	else {
		LOG(ERROR) << "No original WND PROC for this HWND";
		return 0;
	}
}

void ADD_WND_PROC(HWND hWnd, WNDPROC proc)
{
	(*ORIGINAL_WND_PROC_MAP)[hWnd] = proc;
}