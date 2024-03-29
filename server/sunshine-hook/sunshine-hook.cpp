// sunshine-hook.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#define WIN32_LEAN_AND_MEAN
#include "Bootstrap.h"
#include "..\easyloggingpp\easylogging++.h"
#include <easyhook.h>


#if _WIN64
#pragma comment(lib, "EasyHook64.lib")
#else
#pragma comment(lib, "EasyHook32.lib")
#endif

INITIALIZE_EASYLOGGINGPP

#pragma comment(lib, "Ws2_32.lib")

extern "C" void __declspec(dllexport) __stdcall NativeInjectionEntryPoint(REMOTE_ENTRY_INFO* inRemoteInfo);

void __stdcall NativeInjectionEntryPoint(REMOTE_ENTRY_INFO* info)
{
	LOG(INFO) << "Injection ENTRY POINT...";
	auto bootstrap = std::make_unique<Bootstrap>();
	bootstrap->Init(info);
}
