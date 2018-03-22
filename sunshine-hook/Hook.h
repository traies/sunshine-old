#pragma once 
#include <string>
#include <boost\asio.hpp>
#include "EncodePipeline.h"
#include <easyhook.h>
#include "..\easyloggingpp\easylogging++.h"
#include <boost\interprocess\ipc\message_queue.hpp>
#include "UDPClient.h"

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
protected:
	Hook() {};
	HANDLE pipe;
	
	std::shared_ptr<boost::interprocess::message_queue> outputMq;
	bool InstallHook(std::string name, void * oldfunc, void *newfunc)
	{
		ULONG threadIds[] = { 0 };
		HOOK_TRACE_INFO hookTraceInfo;
		ZeroMemory(&hookTraceInfo, sizeof(hookTraceInfo));
		//	Remember to save hookTraceInfo, it is important for release
		if (LhInstallHook(oldfunc, newfunc, NULL, &hookTraceInfo) != 0) {
			LOG(ERROR) << "Hook for function " << name << " failed.";
			return false;
		}

		if (LhSetExclusiveACL(threadIds, 1, &hookTraceInfo)) {
			LOG(ERROR) << "Cannot activate hook for function " << name;
			return false;
		}

		LOG(INFO) << "Hook " << name << " name.";
		return true;
	}
private:
	std::shared_ptr<std::thread> bootstrap;
};

