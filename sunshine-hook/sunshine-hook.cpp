// dllmain.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "Bootstrap.h"

#include "..\easyloggingpp\easylogging++.h"
//#if _WIN64
//#pragma comment(lib, "EasyHook64.lib")
//#else
//#pragma comment(lib, "EasyHook32.lib")
//#endif

INITIALIZE_EASYLOGGINGPP


#pragma comment(lib, "Ws2_32.lib")

//static DWORD WINAPI main_capture_thread(HANDLE thread_handle)
//{
//	RemoteProcessStartInfo* info = new RemoteProcessStartInfo;
//	strcpy_s(info->encoderAPI, "AMD");
//	strcpy_s(info->videoIP, "127.0.0.1");
//	info->videoPort = 1234;
//	auto bootstrap = std::make_unique<Bootstrap>();
//	bootstrap->Init(info);
//	return 0;
//}

//BOOL APIENTRY DllMain(HMODULE hDLL, DWORD Reason, LPVOID Reserved) {
//	/* open file */
//	switch (Reason) {
//	case DLL_PROCESS_ATTACH: {
//		wchar_t name[MAX_PATH];
//
//		HANDLE cur_thread;
//		bool success = DuplicateHandle(GetCurrentProcess(),
//			GetCurrentThread(),
//			GetCurrentProcess(), &cur_thread,
//			SYNCHRONIZE, false, 0);
//
//		/* this prevents the library from being automatically unloaded
//		 * by the next FreeLibrary call */
//		GetModuleFileNameW(hDLL, name, MAX_PATH);
//		LoadLibraryW(name);
//
//		capture_thread = CreateThread(
//			NULL, 0, (LPTHREAD_START_ROUTINE)main_capture_thread,
//			(LPVOID)cur_thread, 0, 0);
//		if (!capture_thread) {
//			CloseHandle(cur_thread);
//			return false;
//		}
//		break;
//	}
//	case DLL_PROCESS_DETACH:
//		//LOG(INFO) << "DLL detach function called.";
//		break;
//	case DLL_THREAD_ATTACH:
//		//LOG(INFO) << "DLL thread attach function called.";
//		break;
//	case DLL_THREAD_DETACH:
//		//LOG(INFO) << "DLL thread detach function called.";
//		break;
//	}
//
//	/* close file */
//	return TRUE;
//}

BOOL APIENTRY DllMain(HMODULE hDLL, DWORD Reason, LPVOID Reserved) {
	/* open file */
	switch (Reason) {
	case DLL_PROCESS_ATTACH: {
		break;
	}
	case DLL_PROCESS_DETACH:
		//LOG(INFO) << "DLL detach function called.";
		break;
	case DLL_THREAD_ATTACH:
		//LOG(INFO) << "DLL thread attach function called.";
		break;
	case DLL_THREAD_DETACH:
		//LOG(INFO) << "DLL thread detach function called.";
		break;
	}

	/* close file */
	return TRUE;
}