#pragma once 
#include <string>
#include <boost\asio.hpp>
#include "EncodePipeline.h"
#include "..\easyloggingpp\easylogging++.h"
#include <boost\interprocess\ipc\message_queue.hpp>
#include "UDPClient.h"
#include "MinHook.h"

//#ifdef _WIN64
//#pragma comment(lib, "MinHook.x64.lib")
//#else
//#pragma comment(lib, "MinHook.x86.lib")
//#endif

LRESULT __stdcall TempWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


class Hook
{
public:
	
	virtual ~Hook() {};
	virtual bool Install() = 0;
	virtual bool Uninstall() = 0;
	
	void SetPipe(HANDLE p)
	{
		pipe = p;
	}
	
	void SetBootstrap(std::shared_ptr<std::thread> b)
	{
		bootstrap = b;
	}

	bool InstallHook(std::string name, void * oldfunc, void *newfunc)
	{
		// Initialize MinHook.
		//if (MH_Initialize() != MH_OK)
		//{
		//	return 1;
		//}

		//// Create a hook for MessageBoxW, in disabled state.
		//if (MH_CreateHook(oldfunc, newfunc, nullptr) != MH_OK)
		//{
		//	return 1;
		//}

		//// Enable the hook for MessageBoxW.
		//if (MH_EnableHook(oldfunc) != MH_OK)
		//{
		//	return 1;
		//}

		//	Remember to save hookTraceInfo, it is important for release
		/*if (LhInstallHook(oldfunc, newfunc, NULL, &hookTraceInfo) != 0) {
			LOG(ERROR) << "Hook for function " << name << " failed.";
			return false;
		}

		if (LhSetExclusiveACL(threadIds, 1, &hookTraceInfo)) {
			LOG(ERROR) << "Cannot activate hook for function " << name;
			return false;
		}*/

		LOG(INFO) << "Hook " << name << " name.";
		return true;
	}
protected:
	Hook() {};
	HANDLE pipe;
	std::shared_ptr<boost::interprocess::message_queue> outputMq;
	
private:
	std::shared_ptr<std::thread> bootstrap;
};

