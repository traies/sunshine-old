// sunshine.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <Psapi.h>
#include "..\easyloggingpp\easylogging++.h"

#define MAX_PROCESSES 2048

INITIALIZE_EASYLOGGINGPP

bool CheckIfProcessIsTargetExe(DWORD pid, LPCWSTR targetExe) {
	HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	
	if (process != nullptr) {
		HMODULE hMod;
		DWORD cbNeeded;
		WCHAR compareString[MAX_PATH];
		if (EnumProcessModules(process, &hMod, sizeof(hMod), &cbNeeded)) {
			GetModuleBaseName(process, hMod, compareString, sizeof(compareString) / sizeof(WCHAR));
			if (lstrcmp(targetExe, compareString) == 0) {
				return true;
			}
		}
	}
	return false;
}


// From https://msdn.microsoft.com/en-us/library/windows/desktop/ms682623(v=vs.85).aspx
bool GetProcessByExe(LPCWSTR targetExe, DWORD * pid)
{
	DWORD processes[MAX_PROCESSES];
	DWORD sizeFound;
	if (!EnumProcesses(processes, sizeof(processes), &sizeFound)) {
		// No processes found (?)
		return false;
	}
	DWORD processesRunning = sizeFound / sizeof(DWORD);
	for (DWORD i = 0; i < processesRunning; i++) {
		if (processes[i] != 0 && CheckIfProcessIsTargetExe(processes[i],targetExe)) {
			// Found targetExe
			*pid = processes[i];
			return true;
		}
	}
	return false;
}


int main()
{
	LOG(INFO) << "Sunshine is starting";
	WCHAR* targetProcessPath = (WCHAR *)L"..\\x64\\Debug\\DXD9TestApplication.exe";
	LPCWSTR targetExe = L"DXD9TestApplication.exe";
	DWORD pid;
	if (GetProcessByExe(targetExe, &pid)) {
		LOG(INFO) << "Process is running with pid: " << pid;
	}
	else {
		LOG(INFO) << "Process is not running.";
	}
    return 0;
}

