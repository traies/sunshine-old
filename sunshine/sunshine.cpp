// sunshine.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <Psapi.h>
#include "..\easyloggingpp\easylogging++.h"

#define MAX_PROCESSES		2048
#define MAX_TRIES			10
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
	// From https://stackoverflow.com/questions/53208/how-do-i-automatically-destroy-child-processes-in-windows
	/*	Creating JobObject for all processes in the application
	*/
	auto job = CreateJobObject(nullptr, nullptr);
	if (job == nullptr) {
		LOG(ERROR) << "Could not create job object";
		ExitProcess(1);
	}
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };
	jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
	if (!SetInformationJobObject(job, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli))) {
		LOG(ERROR) << "Could not SetInformationJobObject";
		ExitProcess(1);
	}

	//	Target path and name for game executables.
	WCHAR targetProcessPath[500] = TEXT("..\\x64\\Debug\\DXD9TestApplication.exe");
	WCHAR targetExe[500] = TEXT("DXD9TestApplication.exe");
	DWORD pid;

	//	CreateProcess and attach to JobObject
	STARTUPINFO info = { sizeof(info) };
	PROCESS_INFORMATION processInfo;
	if (!CreateProcess(nullptr, targetProcessPath, nullptr, nullptr, TRUE, 0, nullptr, nullptr, &info, &processInfo)) {
		LOG(ERROR) << "Could not CreateProcess";
		ExitProcess(1);
	}
	if (!AssignProcessToJobObject(job, processInfo.hProcess)) {
		LOG(ERROR) << "Could not AssignProcessToObject";
		// Also kill the other process...
		ExitProcess(1);
	}
	//	Close unused resource: other process thread.
	CloseHandle(processInfo.hThread);

	LOG(INFO) << "Other process should close now";
	
	/*	Find running process from its exe name; this is 
	*	future proofing. The main process is supposed to
	*	monitor the hooked process and re-attach if the 
	*	process fails to return a heartbeat. This is specially
	*	important for Steam games, since they usually restart
	*	after creation once they realize steam_api.dll is not 
	*	loaded.
	*/
	int tries = 0;
	for (; tries < MAX_TRIES ; tries++) {
		if (GetProcessByExe(targetExe, &pid)) {
			LOG(INFO) << "Process is running with pid: " << pid;
			break;
		}
		else {
			// spin-locking, sould not be that bad.
			LOG(INFO) << "Process is not running... try " << tries + 1 << " of " << MAX_TRIES;
			Sleep(1000);
		}
	}
	if (tries == MAX_TRIES) {
		LOG(ERROR) << "Could not find process after all.";
		ExitProcess(1);
	}
	

	//	Wait for newline, for debugging purposes; should
	//	check constantly for heartbeat instead.
	char whatever[1];
	std::cin.getline(whatever, 1);
	ExitProcess(0);
    return 0;
}

