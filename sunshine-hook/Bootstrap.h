#pragma once
#include "Hook.h"
#include "D9Hook.h"
#include <thread>

class Bootstrap
{
private:
	void InitLogger();
	bool InitOutputPipe();
	void InstallHookD9();
	void InstallHookD11();
	void ConnectHeartbeatPipe();
	void HeartbeatSend(HANDLE file);
	std::thread heartbeat;
	HANDLE pipe;
public:
	Bootstrap();
	~Bootstrap();
	void Init();
};

