#include "stdafx.h"
#include "Bootstrap.h"
#include "D11Hook.h"
#include "..\easyloggingpp\easylogging++.h"
#include <thread>

#define DEFAULT_FIFO_HEARTBEAT	TEXT("\\\\.\\pipe\\sunshine-heartbeat")

Bootstrap::Bootstrap()
{
}


Bootstrap::~Bootstrap()
{

}

void Bootstrap::Init() {
	InitLogger();
	ConnectHeartbeatPipe();
	if (!InitOutputPipe()) {
		LOG(ERROR) << "InitOutputPipe failed.";
		return;
	}
	InstallHookD9();
	InstallHookD11();
	
	heartbeat.join();
	return;
}

void Bootstrap::InitLogger() {
	//	Set logger configuration. Remember: logger output is to be seen on main process.
	el::Configurations defaultConf;
	defaultConf.setToDefault();
	defaultConf.set(el::Level::Global, el::ConfigurationType::Format, "%datetime [%level] [%fbase:%line] %msg");
	el::Loggers::reconfigureLogger("default", defaultConf);
	LOG(DEBUG) << "The function was hooked!";
}

void Bootstrap::InstallHookD9()
{
	auto hook = D9Hook::GetInstance();
	hook->SetPipe(pipe);
	auto ins = hook->Install();
	if (!ins) {
		LOG(ERROR) << "D9Hook installation failed.";
		ExitProcess(1);
	}
}

void Bootstrap::InstallHookD11()
{
	auto hook = D11Hook::GetInstance();
	hook->SetPipe(pipe);
	auto ins = hook->Install();
	if (!ins) {
		LOG(ERROR) << "D11Hook installation failed.";
		ExitProcess(1);
	}
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

void Bootstrap::ConnectHeartbeatPipe()
{
	auto res = CreateFile(DEFAULT_FIFO_HEARTBEAT, GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (res == INVALID_HANDLE_VALUE) {
		LOG(ERROR) << "Heartbeat named pipe could not be connected";
		return;
	}
	LOG(INFO) << "Heartbeat fifo connected.";

	
	heartbeat = std::thread(&Bootstrap::HeartbeatSend, this, res);
}

void Bootstrap::HeartbeatSend(HANDLE file)
{
	uint8_t buff[1];
	buff[0] = 1;
	while (true) {
		DWORD written;
		auto res = WriteFile(file, buff, 1, &written, nullptr);
		if (res && written > 0) {
			Sleep(1000);
			continue;
		}
		else {
			break;
		}
	}
	LOG(INFO) << "Heartbeat send ended.";
	
}