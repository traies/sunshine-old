#pragma once 
#include <string>
#include <easyhook.h>

LRESULT CALLBACK TempWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

class Hook
{
public:
	
	virtual ~Hook() {};
	virtual bool Install() = 0;
	virtual bool Uninstall() = 0;
protected:
	Hook() {};
	bool InstallHook(std::string name, void * oldfunc, void *newfunc);
};

