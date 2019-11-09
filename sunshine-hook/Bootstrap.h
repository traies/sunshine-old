#pragma once
#include "Hook.h"
#include <boost\interprocess\ipc\message_queue.hpp>
#include "D9Hook.h"
#include "D11Hook.h"
#include <RemoteStartupInfo.h>
#include <thread>
#include "UDPClientNew.h"
#include "InputPipeline.h"
#include "Encoder.h"

#define DEFAULT_FIFO_HEARTBEAT	"sunshine_heartbeat"
class Bootstrap
{
private:
	void InitLogger();
	std::shared_ptr<UDPClientNew> InitSocket();
	void InstallHookD9();
	void InstallHookD11();
	void InstallHookOpenGL();
	void InitInputPipeline();
	void HeartbeatSend(std::shared_ptr<boost::interprocess::message_queue> mq);
	
	bool stop = false;
	std::shared_ptr<UDPClientNew> socket;
	std::shared_ptr<std::thread> heartbeat;
	std::shared_ptr<boost::interprocess::message_queue> mq;
	static std::unique_ptr<InputPipeline> inputPipeline;
	RemoteProcessStartInfo* _startupInfo = nullptr;
public:
	Bootstrap() : mq(std::make_shared<boost::interprocess::message_queue>(boost::interprocess::open_only, DEFAULT_FIFO_HEARTBEAT)) {};
	~Bootstrap() {};
	void Init(REMOTE_ENTRY_INFO * info);

	std::shared_ptr<UDPClientNew> GetSocket() { return socket; };
	std::shared_ptr<std::thread> GetHeartbeat() { return heartbeat; };
};

