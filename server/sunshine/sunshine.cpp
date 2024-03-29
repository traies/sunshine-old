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
#include <RemoteStartupInfo.h>
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

//mpv.exe --no-cache --vo=gpu --gpu-api=opengl --gpu-context=win --swapchain-depth=1 --video-latency-hacks=yes --hwdec=yes --untimed --no-demuxer-thread --vd-lavc-threads=1  "udp://127.0.0.1:1234"
//mpv --no-cache --vo=gpu --gpu-api=d3d11 --gpu-context=d3d11 --hwdec=d3d11va --d3d11-sync-interval=0 --d3d11va-zero-copy=yes --untimed --no-demuxer-thread --vd-lavc-threads=1 "tcp://127.0.0.1:1234?listen"
#define MAX_PROCESSES		2048
#define MAX_TRIES			100
#define DEFAULT_FIFO_DEBUG	"sunshine_debug"
#define DEFAULT_FIFO_HEARTBEAT	"sunshine_heartbeat"

INITIALIZE_EASYLOGGINGPP

const char* GetOption(char* argv[], int argc, const char* option, const char* def) {
	const char* value = nullptr;
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], option) == 0) {
			value = argv[i + 1];
			break;
		}
	}
	if (value == nullptr) {
		value = def;
	}
	printf("%-10s=\t%s\n", option, value);
	return value;
}


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

void HeartbeatThread(LPCWSTR targetExe, HANDLE job, RemoteProcessStartInfo startupInfo)
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
		auto nstatus = RhInjectLibrary(pid, 0, EASYHOOK_INJECT_DEFAULT, hookDllx86, hookDllx64, &startupInfo, sizeof(RemoteProcessStartInfo));
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
						//TODO REMOVE
						break;
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

int main(int argc, char * argv[])
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
	const char* path = GetOption(argv, argc, "--path", nullptr);
	if (!path) {
		LOG(ERROR) << "No path provided.";
		ExitProcess(1);
	}

	std::string strPathAux(path);
	std::wstring targetProcessPath(strPathAux.begin(), strPathAux.end());
	size_t lastSlash = targetProcessPath.rfind(L"\\");
	if (lastSlash == std::wstring::npos) {
		LOG(ERROR) << "Provide the full path for the application using \\.";
		ExitProcess(1);
	}

	const char* start = GetOption(argv, argc, "--start", "true");

	if (strcmp(start, "true") == 0) {
		const char * steamAppId = GetOption(argv, argc, "--steam", nullptr);
		if (steamAppId != nullptr) {
			std::string steamCommand("start steam://rungameid/");
			steamCommand.append(steamAppId);
			system(steamCommand.c_str());
		} else {

			//	CreateProcess and attach to JobObject
			STARTUPINFO info;
			ZeroMemory(&info, sizeof(info));

			PROCESS_INFORMATION processInfo;
		
			//	Target path and directory for game executables.
			LPWSTR targetProcessFullPathW = new WCHAR[targetProcessPath.size() + 1];
			wcscpy_s(targetProcessFullPathW, targetProcessPath.size() + 1, targetProcessPath.c_str());

			auto targetProcessDirectory = targetProcessPath.substr(0, lastSlash);
			LPWSTR targetProcessDirectoryW = new WCHAR[targetProcessDirectory.size() + 1];
			wcscpy_s(targetProcessDirectoryW, targetProcessDirectory.size() + 1, targetProcessDirectory.c_str());

			if (!CreateProcess(nullptr, targetProcessFullPathW, nullptr, nullptr, TRUE, 0, nullptr, targetProcessDirectoryW, &info, &processInfo)) {
				LOG(ERROR) << "Could not CreateProcess. " << GetLastError();
				ExitProcess(1);
			}
			//	Close unused resource: other process thread.
			CloseHandle(processInfo.hProcess);
			CloseHandle(processInfo.hThread);
			delete[] targetProcessFullPathW;
			delete[] targetProcessDirectoryW;
		}
	}
	auto debugMq = InitDebugMessageQueue();

	auto targetExe = targetProcessPath.substr(lastSlash + 1, targetProcessPath.size() - lastSlash + 1);

	const char* videoIP = GetOption(argv, argc, "--ip", "127.0.0.1");
	int videoPort = atoi(GetOption(argv, argc, "--port", "1234"));
	const char* encoderAPI = GetOption(argv, argc, "--encoder", "AMD");
	RemoteProcessStartInfo startupInfo;
	if (strcpy_s(startupInfo.videoIP, sizeof(startupInfo.videoIP), videoIP) != 0) {
		LOG(ERROR) << "Could not read videoIP ";
		ExitProcess(0);
	}
	if (strcpy_s(startupInfo.encoderAPI, sizeof(startupInfo.encoderAPI), encoderAPI)) {
		LOG(ERROR) << "Could not read encoderAPI";
		ExitProcess(0);
	}
	startupInfo.videoPort = videoPort;
	startupInfo.bitrate = atoi(GetOption(argv, argc, "--bitrate", "10000000"));
	const char * codec = GetOption(argv, argc, "--codec", "H264");
	if (strcmp(codec, "HEVC") == 0) {
		startupInfo.codec = CODEC::HEVC;
	}
	else {
		startupInfo.codec = CODEC::H264;
	}
	const char* noControl = GetOption(argv, argc, "--no-control", "false");
	if (strcmp(noControl, "true") == 0) {
		startupInfo.noControl = true;
	}
	else {
		startupInfo.noControl = false;
	}
	//	Inject target process and check heartbeat.
	std::thread heartbeatThread(HeartbeatThread, targetExe.c_str(), job, startupInfo);

	// Start the debugThread and join (exit when it returns).
	std::thread debugThread(DebugThread, std::move(debugMq));
	debugThread.detach();
	heartbeatThread.join();
	ExitProcess(0);
}

