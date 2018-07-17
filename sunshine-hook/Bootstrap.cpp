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

#define DEFAULT_FIFO_DEBUG	"sunshine_debug"

std::shared_ptr<UDPClient> Bootstrap::InitSocket()
{
	return std::make_shared<UDPClient>("127.0.0.1", 1234);
}

void Bootstrap::Init() {
	InitLogger();
	heartbeat = std::make_shared<std::thread>(std::thread(&Bootstrap::HeartbeatSend, this, mq));
	
	socket = InitSocket();
	/*if (!InitOutputPipe()) {
		LOG(ERROR) << "InitOutputPipe failed.";
		return;
	}*/
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
#ifdef NVIDIA_ENC
	auto hook = D9Hook<Encode::NvidiaEncoder>::GetInstance();
#endif // NVIDIA_ENC

#ifndef NVIDIA_ENC
	auto hook = D9Hook<Encode::AmdEncoder>::GetInstance();
#endif
	
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
#ifdef NVIDIA_ENC
	auto hook = D11Hook<Encode::NvidiaEncoder>::GetInstance();
#endif // NVIDIA_ENC

#ifndef NVIDIA_ENC
	auto hook = D11Hook<Encode::AmdEncoder>::GetInstance();
#endif
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
	auto hook = GLHook<Encode::AmdEncoder>::GetInstance();
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