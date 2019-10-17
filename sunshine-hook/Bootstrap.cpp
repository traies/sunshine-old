#include "stdafx.h"
#include "Bootstrap.h"
#include "D11Hook.h"
#include "GLHook.h"
#include "..\easyloggingpp\easylogging++.h"
#include <thread>
#include "MessageQueueDispatcher.h"
#include "InputPipeline.h"
#include "AmdEncoder.h"
#include "NvidiaEncoder.h"
#include "FocusHook.h"
#include "Encoder.h"
#include "ControlHook.h"

#define DEFAULT_FIFO_DEBUG	"sunshine_debug"

std::shared_ptr<UDPClient> Bootstrap::InitSocket()
{
	LOG(INFO) << "Streaming video to " << _startupInfo->videoIP << ":" << _startupInfo->videoPort;
	return std::make_shared<UDPClient>(_startupInfo->videoIP, _startupInfo->videoPort);
}

void Bootstrap::Init(REMOTE_ENTRY_INFO * info) {
	InitLogger();
	heartbeat = std::make_shared<std::thread>(std::thread(&Bootstrap::HeartbeatSend, this, mq));
	
	_startupInfo= reinterpret_cast<RemoteProcessStartInfo * >(info->UserData);
	LOG(INFO) << _startupInfo->videoIP;
	LOG(INFO) << _startupInfo->encoderAPI;
	socket = InitSocket();
	InstallHookD9();
	InstallHookD11();
	//InstallHookOpenGL();
	InitInputPipeline();
	
	return;
}

void Bootstrap::InitLogger() {
	//	Install MessageQueueDispatcher
	el::Helpers::installLogDispatchCallback<MessageQueueDispatcher>("MessageQueueDispatcher");
	MessageQueueDispatcher * mqd = el::Helpers::logDispatchCallback<MessageQueueDispatcher>("MessageQueueDispatcher");
	mqd->SetMessageQueue(DEFAULT_FIFO_DEBUG);

	
	// Set logger configuration.Remember: logger output is to be seen on main process.
	el::Configurations defaultConf;
	defaultConf.setToDefault();
	defaultConf.set(el::Level::Global, el::ConfigurationType::Format, "%datetime [%level] [%fbase:%line] %msg");
	el::Loggers::reconfigureLogger("default", defaultConf);	
	LOG(INFO) << "The function was hooked!";
}

void Bootstrap::InstallHookD9()
{
	D9Hook * hook;
	if (strcmp(_startupInfo->encoderAPI, "NVIDIA") == 0) {
		LOG(INFO) << "Using NVIDIA encoder...";
		hook = D9Hook::CreateInstance(std::make_unique<Encode::NvidiaEncoder>());
	}
	else {
		LOG(INFO) << "Using AMD encoder...";
		hook = D9Hook::CreateInstance(std::make_unique<Encode::AmdEncoder>());
	}
	hook->SetSocket(socket);
	hook->SetBootstrap(heartbeat);
	auto ins = hook->Install();
	if (!ins) {
		LOG(ERROR) << "D9Hook installation failed.";
		ExitProcess(1);
	}
}

void Bootstrap::InstallHookD11()
{
	D11Hook * hook = nullptr;
	if (strcmp(_startupInfo->encoderAPI, "NVIDIA") == 0) {
		LOG(INFO) << "Setting up NVIDIA encoder for D11 hoook...";
		hook = D11Hook::CreateInstance(std::make_unique<Encode::NvidiaEncoder>());
	}
	else {
		LOG(INFO) << "Setting up AMD encoder for D11 hook...";
		hook = D11Hook::CreateInstance(std::make_unique<Encode::AmdEncoder>());
	}
	hook->SetSocket(socket);
	hook->SetBootstrap(heartbeat);
	auto ins = hook->Install();
	if (!ins) {
		LOG(ERROR) << "D11Hook installation failed.";
		ExitProcess(1);
	}
}

void Bootstrap::InstallHookOpenGL()
{
	auto hook = GLHook::GetInstance();
	if (strcmp(_startupInfo->encoderAPI, "NVIDIA") == 0) {
		LOG(INFO) << "Setting up NVIDIA encoder for GL hoook...";
		hook = GLHook::CreateInstance(std::make_unique<Encode::NvidiaEncoder>());
	}
	else {
		LOG(INFO) << "Setting up AMD encoder for GL hook...";
		hook = GLHook::CreateInstance(std::make_unique<Encode::AmdEncoder>());
	}
	hook->SetSocket(socket);
	hook->SetBootstrap(heartbeat);
	auto ins = hook->Install();
	if (!ins) {
		LOG(ERROR) << "GLHook installation failed.";
		ExitProcess(1);
	}
}

std::unique_ptr<InputPipeline> Bootstrap::inputPipeline;
void Bootstrap::InitInputPipeline()
{
	inputPipeline = std::make_unique<InputPipeline>();
	std::thread thread(&InputPipeline::Run, std::ref(*inputPipeline));
	thread.detach();
}

void Bootstrap::HeartbeatSend(std::shared_ptr<boost::interprocess::message_queue> mq)
{
	while (true) {
		uint8_t buff = 1;
		mq->send(&buff, sizeof(buff), 0);
	}
	LOG(INFO) << "Heartbeat send ended.";
	
}