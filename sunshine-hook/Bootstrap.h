#pragma once
#include "Hook.h"
#include "D9Hook.h"
#include <thread>
#include <boost\interprocess\ipc\message_queue.hpp>
#define DEFAULT_FIFO_HEARTBEAT	"sunshine_heartbeat"
class Bootstrap
{
private:
	void InitLogger();
	bool InitOutputPipe();
	void InstallHookD9();
	void InstallHookD11();
	void InstallHookOpenGL();
	void HeartbeatSend();
	std::thread heartbeat;
	HANDLE pipe;
	boost::interprocess::message_queue mq;
public:
	Bootstrap() : mq(boost::interprocess::open_only, DEFAULT_FIFO_HEARTBEAT) {};
	~Bootstrap();
	void Init();
};

