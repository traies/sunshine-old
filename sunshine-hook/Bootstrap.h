#pragma once
#include "Hook.h"
#include <boost\interprocess\ipc\message_queue.hpp>
#include "D9Hook.h"
#include <thread>
#include "UDPClient.h"

#define DEFAULT_FIFO_HEARTBEAT	"sunshine_heartbeat"
class Bootstrap
{
private:
	void InitLogger();
	auto InitSocket();
	bool InitOutputPipe();
	void InstallHookD9();
	void InstallHookD11();
	void InstallHookOpenGL();
	void HeartbeatSend(std::shared_ptr<boost::interprocess::message_queue> mq);
	
	HANDLE pipe;
	bool stop = false;
	std::shared_ptr<UDPClient> socket;
	std::shared_ptr<std::thread> heartbeat;
	std::shared_ptr<boost::interprocess::message_queue> mq;
public:
	Bootstrap() : mq(std::make_shared<boost::interprocess::message_queue>(boost::interprocess::open_only, DEFAULT_FIFO_HEARTBEAT)) {};
	~Bootstrap();
	void Init();

	std::shared_ptr<UDPClient> GetSocket() { return socket; };
	std::shared_ptr<std::thread> GetHeartbeat() { return heartbeat; };
};

