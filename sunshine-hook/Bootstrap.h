#pragma once
#include "Hook.h"
#include <boost\interprocess\ipc\message_queue.hpp>
#include "D9Hook.h"
#include <thread>
#include "UDPClient.h"
#include "InputPipeline.h"

//#define NVIDIA_ENC

#define DEFAULT_FIFO_HEARTBEAT	"sunshine_heartbeat"
class Bootstrap
{
private:
	void InitLogger();
	auto InitSocket();
	void InstallHookD9();
	void InstallHookD11();
	void InstallHookOpenGL();
	void InitInputPipeline();
	void HeartbeatSend(std::shared_ptr<boost::interprocess::message_queue> mq);
	
	bool stop = false;
	std::shared_ptr<UDPClient> socket;
	std::shared_ptr<std::thread> heartbeat;
	std::shared_ptr<boost::interprocess::message_queue> mq;
	static std::unique_ptr<InputPipeline> inputPipeline;
public:
	Bootstrap() : mq(std::make_shared<boost::interprocess::message_queue>(boost::interprocess::open_only, DEFAULT_FIFO_HEARTBEAT)) {};
	~Bootstrap();
	void Init();

	std::shared_ptr<UDPClient> GetSocket() { return socket; };
	std::shared_ptr<std::thread> GetHeartbeat() { return heartbeat; };
};

