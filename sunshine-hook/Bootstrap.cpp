#include "stdafx.h"
#include "Bootstrap.h"
#include "..\easyloggingpp\easylogging++.h"

Bootstrap::Bootstrap()
{
}


Bootstrap::~Bootstrap()
{

}

void Bootstrap::Init() {
	InitLogger();
	InstallHookD9();
	//InstallHookD11();
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
	auto ins = hook->Install();
	if (!ins) {
		LOG(ERROR) << "D9Hook installation failed.";
		ExitProcess(1);
	}
}

void Bootstrap::InstallHookD11()
{

}