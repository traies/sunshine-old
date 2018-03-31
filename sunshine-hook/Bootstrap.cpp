#include "stdafx.h"
#include "Bootstrap.h"
#include "D11Hook.h"
#include "GLHook.h"
#include "..\easyloggingpp\easylogging++.h"
#include <thread>
#include "MessageQueueDispatcher.h"
#include "InputPipeline.h"
#include "AmdEncoder.h"

#define DEFAULT_FIFO_DEBUG	"sunshine_debug"
Bootstrap::~Bootstrap()
{

}

auto Bootstrap::InitSocket()
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
	InstallHookOpenGL();
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
	auto hook = D9Hook<Encode::AmdEncoder>::GetInstance();
	hook->SetPipe(pipe);
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
	auto hook = D11Hook<Encode::AmdEncoder>::GetInstance();
	hook->SetPipe(pipe);
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
	hook->SetPipe(pipe);
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
}

bool Bootstrap::InitOutputPipe()
{
	//	THIS IS WRONG. Pipe should not be created here, and pipe path should be a passed parameter.
	//	For now, this works in order to check that the AMF encoder works properly.
	//	The process will block until someone connects on the other side.
	//	Use the following command:
	//	ffmpeg.exe -i \\.\pipe\ffpipe -vcodec libx264 -tune zerolatency -f sdl -preset ultrafast -profile:v baseline -crf 17 -pix_fmt yuv420p -f mpegts udp://127.0.0.1:1234
 	pipe = CreateNamedPipe(TEXT("\\\\.\\pipe\\ffpipe"),
		PIPE_ACCESS_DUPLEX,
		PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,   // FILE_FLAG_FIRST_PIPE_INSTANCE is not needed but forces CreateNamedPipe(..) to fail if the pipe already exists...
		1,
		1024 * 16,
		1024 * 16,
		NMPWAIT_USE_DEFAULT_WAIT,
		NULL);
	if (pipe == INVALID_HANDLE_VALUE) {
		LOG(ERROR) << "Named pipe was invalid.";
		return false;
	}
	int maxTries = 5;
	int tries = 0;
	for (;tries < maxTries; tries++)
	{
		if (ConnectNamedPipe(pipe, NULL) != FALSE)   // wait for someone to connect to the pipe
		{
			break;
		}
		else {
			LOG(INFO) << "Connecting to named pipe failed. Try " << tries << " of " << maxTries;
		}
	}
	if (tries == maxTries) {
		LOG(ERROR) << "Maximun tries failed.";
		return false;
	}
	return true;
}

void Bootstrap::HeartbeatSend(std::shared_ptr<boost::interprocess::message_queue> mq)
{
	while (true) {
		uint8_t buff = 1;
		mq->send(&buff, sizeof(buff), 0);
	}
	LOG(INFO) << "Heartbeat send ended.";
	
}