#pragma once 
#include <string>
#include "EncodePipeline.h"
#include <easyhook.h>
#include "..\easyloggingpp\easylogging++.h"

LRESULT __stdcall TempWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

template <typename PipelineType, typename DeviceType>
class Hook
{
public:
	
	~Hook() {};
	virtual bool Install() = 0;
	virtual bool Uninstall() = 0;
	virtual std::shared_ptr<PipelineType> GetEncodePipeline(DeviceType * device) = 0;
	void SetPipe(HANDLE p)
	{
		pipe = p;
	}
protected:
	Hook() {};
	HANDLE pipe;
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
};

