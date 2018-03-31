// sunshine.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <Psapi.h>
#include <easyhook.h>
#include <stdint.h>
#include <thread>
#include <string.h>
#include "..\EasyHook-2.7.6270.0-Binaries\NetFX4.0\ntstatus.h"
#include "..\easyloggingpp\easylogging++.h"
#include <boost\interprocess\ipc\message_queue.hpp>
#include <boost\date_time\posix_time\posix_time.hpp>
#include <boost\exception\all.hpp>
#include <boost\exception\info.hpp>
#include <boost\exception\get_error_info.hpp>

#ifdef _DEBUG
#define HOOKDLL_x86		"..\\Debug\\sunshine-hook.dll"
#define HOOKDLL_x64		"..\\x64\\Debug\\sunshine-hook.dll"
#endif // DEBUG

#ifndef _DEBUG
#define HOOKDLL_x86		"..\\Release\\sunshine-hook.dll"
#define HOOKDLL_x64		"..\\x64\\Release\\sunshine-hook.dll"
#endif // !DEBUG

#if _WIN64
#pragma comment(lib, "EasyHook64.lib")
#else
#pragma comment(lib, "EasyHook32.lib")
#endif

#if _WIN64
#define TEST_APPLICATION_PATH	TEXT("..\\x64\\Debug\\DXD9TestApplication.exe")
//#define TEST_APPLICATION_PATH	TEXT("E:\\SteamLibrary\\steamapps\\common\\Golf With Your Friends\\Golf With Your Friends.exe")
//#define TEST_APPLICATION_PATH	TEXT("E:\\games\\Hollow Knight The Grimm Troupe\\hollow_knight.exe")
//#define TEST_APPLICATION_PATH	TEXT("C:\\GOG Games\\Cuphead\\Cuphead.exe")
//#define TEST_APPLICATION_PATH	TEXT("E:\\SteamLibrary\\steamapps\\common\\SunlessSea\\Sunless Sea.exe")
//#define TEST_APPLICATION_PATH	TEXT("E:\\SteamLibrary\\steamapps\\common\\Half-Life\\hl.exe")
//#define TEST_APPLICATION_PATH	TEXT("E:\\SteamLibrary\\steamapps\\common\\Alien Isolation\\AI.exe")
//#define TEST_APPLICATION_PATH	TEXT("E:\\SteamLibrary\\steamapps\\common\\Left 4 Dead 2\\left4dead2.exe")
//#define TEST_APPLICATION_PATH	TEXT("E:\\SteamLibrary\\steamapps\\common\\Half-Life 2\\hl2.exe")
//#define TEST_APPLICATION_PATH	TEXT("E:\\SteamLibrary\\steamapps\\common\\DOOM\\DOOMx64.exe")
//#define TEST_APPLICATION_PATH	TEXT("E:\\SteamLibrary\\steamapps\\common\\dota 2 beta\\game\\bin\\win64\\dota2.exe -vulkan")
//#define TEST_APPLICATION_PATH	TEXT("E:\\games\\Fallout 4\\Fallout4.exe")
//#define TEST_APPLICATION_PATH	TEXT("E:\\games\\FINAL.FANTASY.XV.WINDOWS.EDITION.Incl.DLCs\\FINAL.FANTASY.XV.WINDOWS.EDITION.Incl.DLCs\\FINAL FANTASY XV\\ffxv_s.exe")
//#define TEST_APPLICATION_PATH	TEXT("E:\\SteamLibrary\\steamapps\\common\\Portal\\hl2.exe")



#else
#define TEST_APPLICATION_PATH	TEXT("..\\Debug\\DXD9TestApplication.exe")
//#define TEST_APPLICATION_PATH	TEXT("E:\\SteamLibrary\\steamapps\\common\\Golf With Your Friends\\Golf With Your Friends.exe")
//#define TEST_APPLICATION_PATH	TEXT("E:\\games\\Hollow Knight The Grimm Troupe\\hollow_knight.exe")
//#define TEST_APPLICATION_PATH	TEXT("C:\\GOG Games\\Cuphead\\Cuphead.exe")
//#define TEST_APPLICATION_PATH	TEXT("E:\\SteamLibrary\\steamapps\\common\\SunlessSea\\Sunless Sea.exe")
//#define TEST_APPLICATION_PATH	TEXT("E:\\SteamLibrary\\steamapps\\common\\Half-Life\\hl.exe")
//#define TEST_APPLICATION_PATH	TEXT("E:\\SteamLibrary\\steamapps\\common\\Alien Isolation\\AI.exe")
//#define TEST_APPLICATION_PATH	TEXT("E:\\SteamLibrary\\steamapps\\common\\Left 4 Dead 2\\left4dead2.exe")
//#define TEST_APPLICATION_PATH	TEXT("E:\\SteamLibrary\\steamapps\\common\\Half-Life 2\\hl2.exe")
//#define TEST_APPLICATION_PATH	TEXT("E:\\SteamLibrary\\steamapps\\common\\DOOM\\DOOMx64.exe")
//#define TEST_APPLICATION_PATH	TEXT("E:\\SteamLibrary\\steamapps\\common\\dota 2 beta\\game\\bin\\win64\\dota2.exe -gl")
//#define TEST_APPLICATION_PATH	TEXT("E:\\games\\Fallout 4\\Fallout4.exe")a
//#define TEST_APPLICATION_PATH	TEXT("E:\\games\\FINAL.FANTASY.XV.WINDOWS.EDITION.Incl.DLCs\\FINAL.FANTASY.XV.WINDOWS.EDITION.Incl.DLCs\\FINAL FANTASY XV\\ffxv_s.exe")
//#define TEST_APPLICATION_PATH	TEXT("E:\\SteamLibrary\\steamapps\\common\\Portal\\hl2.exe")

#endif

#define TEST_APPLICATION_NAME	TEXT("DXD9TestApplication.exe")
//#define TEST_APPLICATION_NAME	TEXT("Sunless Sea.exe")
//#define TEST_APPLICATION_NAME	TEXT("Golf With Your Friends.exe")
//#define TEST_APPLICATION_NAME	TEXT("hollow_knight.exe")
//#define TEST_APPLICATION_NAME	TEXT("AI.exe")
//#define TEST_APPLICATION_NAME	TEXT("left4dead2.exe")
//#define TEST_APPLICATION_NAME	TEXT("hl2.exe")
//#define TEST_APPLICATION_NAME	TEXT("DOOMx64.exe")
//#define TEST_APPLICATION_NAME	TEXT("dota2.exe")
//#define TEST_APPLICATION_NAME	TEXT("hl.exe")
//#define TEST_APPLICATION_NAME	TEXT("Fallout4.exe")
//#define TEST_APPLICATION_NAME	TEXT("ffxv_s.exe")
//#define TEST_APPLICATION_NAME	TEXT("Cuphead.exe")

#define TEST_APPLICATION_DIRECTORY	TEXT("..\\Debug\\")
//#define TEST_APPLICATION_DIRECTORY	TEXT("E:\\games\\Hollow Knight The Grimm Troupe")
//#define TEST_APPLICATION_DIRECTORY	TEXT("E:\\games\\Fallout 4")
//#define TEST_APPLICATION_DIRECTORY		TEXT("E:\\SteamLibrary\\steamapps\\common\\Half-Life")
//#define TEST_APPLICATION_DIRECTORY	TEXT("E:\\SteamLibrary\\steamapps\\common\\Alien Isolation")
//#define TEST_APPLICATION_DIRECTORY	TEXT("E:\\SteamLibrary\\steamapps\\common\\Left 4 Dead 2")
//#define TEST_APPLICATION_DIRECTORY	TEXT("E:\\SteamLibrary\\steamapps\\common\\Half-Life 2")
//#define TEST_APPLICATION_DIRECTORY	TEXT("E:\\SteamLibrary\\steamapps\\common\\Golf With Your Friends")
//#define TEST_APPLICATION_DIRECTORY	TEXT("E:\\SteamLibrary\\steamapps\\common\\SunlessSea")
//#define TEST_APPLICATION_DIRECTORY	TEXT("E:\\games\\FINAL.FANTASY.XV.WINDOWS.EDITION.Incl.DLCs\\FINAL.FANTASY.XV.WINDOWS.EDITION.Incl.DLCs\\FINAL FANTASY XV")
//#define TEST_APPLICATION_DIRECTORY	TEXT("C:\\GOG Games\\Cuphead")
//#define TEST_APPLICATION_DIRECTORY	TEXT("E:\\SteamLibrary\\steamapps\\common\\Portal")S


//mpv.exe --no-cache --vo=gpu --gpu-api=opengl --gpu-context=win --swapchain-depth=1 --video-latency-hacks=yes --hwdec=yes --untimed --no-demuxer-thread --vd-lavc-threads=1  "udp://127.0.0.1:1234"
//mpv.exe --no-cache --vo=gpu --gpu-api=d3d11 --gpu-context=d3d11 --hwdec=d3d11va --d3d11-sync-interval=0 --d3d11va-zero-copy=yes --untimed --no-demuxer-thread --vd-lavc-threads=1 "udp://127.0.0.1:1234"
#define MAX_PROCESSES		2048
#define MAX_TRIES			10
#define DEFAULT_FIFO_DEBUG	"sunshine_debug"
#define DEFAULT_FIFO_HEARTBEAT	"sunshine_heartbeat"

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

// From https://msdn.microsoft.com/en-us/library/windows/desktop/ms682499(v=vs.85).aspx
auto StartDebugPipe(PHANDLE in, PHANDLE out) {
	SECURITY_ATTRIBUTES securityAttributes;
	securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
	securityAttributes.bInheritHandle = TRUE;
	securityAttributes.lpSecurityDescriptor = NULL;
	// Creates a pipe, used to redirect output from child process.
	return CreatePipe(out, in, &securityAttributes, sizeof(securityAttributes));
}

std::unique_ptr<boost::interprocess::message_queue> InitDebugMessageQueue()
{
	//	Remove Message queue if present.
	boost::interprocess::message_queue::remove(DEFAULT_FIFO_DEBUG);
	try {
		auto mq = std::make_unique<boost::interprocess::message_queue>(boost::interprocess::create_only, DEFAULT_FIFO_DEBUG, 5, sizeof(uint8_t) * 1000);
		return std::move(mq);
	}
	catch (boost::exception const &e) {
		ExitProcess(1);
	}
}

int DebugThread(std::unique_ptr<boost::interprocess::message_queue> mq) {
	// Start a custom logger (we don't want info regarding this thread, just the remote one)
	auto debugLogger = el::Loggers::getLogger("DebugLogger");
	auto config = debugLogger->configurations();
	config->set(el::Level::Global, el::ConfigurationType::Format, "Hook says: %msg");
	debugLogger->configure(*config);

	//	Blocks reading from the remote pipe. If it fails or the other end closes, the process
	//	exit. This behaviour should be removed outside of Debug.
	uint8_t buffer[1000], safebuffer[1000];
	boost::interprocess::message_queue::size_type read;
	unsigned int priority;
	while (true) {
		mq->receive(buffer, 1000, read, priority);
		auto buf = strncpy_s((char *) safebuffer, 1000, (char *) buffer, read);
		debugLogger->debug(safebuffer);
	}
	LOG(ERROR) << "Debug fifo closed.";
	return 1;
}

std::unique_ptr<boost::interprocess::message_queue> InitHeartbeat()
{
	//	Remove Message queue if present.
	boost::interprocess::message_queue::remove(DEFAULT_FIFO_HEARTBEAT);
	try {
		auto mq = std::make_unique<boost::interprocess::message_queue>(boost::interprocess::create_only, DEFAULT_FIFO_HEARTBEAT, 1, sizeof(uint8_t));
		return std::move(mq);
	}
	catch (boost::exception const &e) {
		ExitProcess(1);
	}
}

void HeartbeatThread(LPCWSTR targetExe, HANDLE job)
{
	auto heartbeatMq = InitHeartbeat();
	
	do {
		/*	Find running process from its exe name; this is
		*	future proofing. The main process is supposed to
		*	monitor the hooked process and re-attach if the
		*	process fails to return a heartbeat. This is specially
		*	important for Steam games, since they usually restart
		*	after creation once they realize steam_api.dll is not
		*	loaded.
		*/
		DWORD pid;
		int tries = 0;
		for (; tries < MAX_TRIES; tries++) {
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
			break;
		}
		auto procHandle = OpenProcess(PROCESS_SET_QUOTA | PROCESS_TERMINATE, TRUE, pid);
		if (procHandle == nullptr) {
			LOG(ERROR) << "Could not get procHandle from OpenProcess.";
			break;
		}
		
		
		//if (!AssignProcessToJobObject(job, procHandle)) {
		//	LOG(ERROR) << "Could not AssignProcessToObject";
		//	// Also kill the other process...
		//	break;
		//}
		
		WCHAR hookDllx86[500] = TEXT(HOOKDLL_x86);
		WCHAR hookDllx64[500] = TEXT(HOOKDLL_x64);

		//	Inject libraries. For some reason, x64 builds fail to inject x86 applications and viceversa, even though 
		//	this should work. Further investigation required.
		auto nstatus = RhInjectLibrary(pid, 0, EASYHOOK_INJECT_DEFAULT, hookDllx86, hookDllx64, nullptr, 0);
		switch (nstatus) {
		case STATUS_INVALID_PARAMETER_5:
			LOG(ERROR) << "Injection failed: The 64 bit library could not be exported, does not contain the expected entry point or could not be loaded into target process.";
			break;
		case STATUS_INVALID_PARAMETER_4:
			LOG(ERROR) << "Injection failed: The 32 bit library could not be exported, does not contain the expected entry point or could not be loaded into target process.";
			break;
		case STATUS_WOW_ASSERTION:
			LOG(ERROR) << "Injection failed: Injection through WOW64 boundaries not supported.";
			break;
		case STATUS_NOT_FOUND:
			LOG(ERROR) << "Injection failed: The target process cound not be found.";
			break;
		case STATUS_ACCESS_DENIED:
			LOG(ERROR) << "Injection failed: The target process could not be accesed properly or remote thread creation failed.";
			break;
		case STATUS_SUCCESS:
			LOG(INFO) << "Injection succeded";
			break;
		default:
			LOG(ERROR) << "Injection failed: Unknown error";
			break;
		}
		// Exit process if injection fails.
		if (nstatus != STATUS_SUCCESS) {
			break;
		}
		
		int maxTries = 5;
		while (true) {
			int tries = 0;
			for (; tries < maxTries; tries++) {
				//	Read a single byte from heartbeat.
				uint8_t dummy;
				unsigned int priority;
				boost::interprocess::message_queue::size_type read = 0;
				try {
					bool received = heartbeatMq->try_receive(&dummy, sizeof(dummy), read, priority);
					Sleep(1000);
					if (received && read == 1) {
						LOG(INFO) << "Hearbeat received.";
						break;
					}
					else {
						LOG(INFO) << "No hearbeat received. Try " << tries + 1 << " of " << maxTries << ".";
					}
				}
				catch (boost::interprocess::interprocess_exception &ex) {
					LOG(ERROR) << "Exception ocurred: " << ex.what();
					break;
				}
				
			}
			if (tries < maxTries) {
				continue;
			}
			else {
				break;
			}
		}
		LOG(INFO) << "Heartbeat failed to respond. Attempting re-attach.";
	} while (true);
	boost::interprocess::message_queue::remove(DEFAULT_FIFO_HEARTBEAT);
	return;
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
	WCHAR targetProcessPath[500] = TEST_APPLICATION_PATH;
	WCHAR targetExe[500] = TEST_APPLICATION_NAME;
	WCHAR targetProcessDirectory[500] = TEST_APPLICATION_DIRECTORY;

	//	Init debug pipe to redirect child process output.
	/*HANDLE in, out;
	if (!StartDebugPipe(&in, &out)) {
		LOG(ERROR) << "Could not start debug pipe.";
		ExitProcess(1);
	}*/

	//	CreateProcess and attach to JobObject
	STARTUPINFO info;
	ZeroMemory(&info, sizeof(info));

	PROCESS_INFORMATION processInfo;
	if (!CreateProcess(nullptr, targetProcessPath, nullptr, nullptr, TRUE, 0, nullptr, targetProcessDirectory, &info, &processInfo)) {
		LOG(ERROR) << "Could not CreateProcess";
		ExitProcess(1);
	}
	//	Close unused resource: other process thread.
	CloseHandle(processInfo.hThread);

	auto debugMq = InitDebugMessageQueue();

	//	Inject target process and check heartbeat.
	std::thread heartbeatThread(HeartbeatThread, targetExe, job);

	// Start the debugThread and join (exit when it returns).
	std::thread debugThread(DebugThread, std::move(debugMq));
	debugThread.detach();
	heartbeatThread.join();
	ExitProcess(0);
}

