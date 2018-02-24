#pragma once
#include "Hook.h"
#include "D9Hook.h"
class Bootstrap
{
private:
	void InitLogger();
	void InitOutputPipe();
	void InstallHookD9();
	void InstallHookD11();
	HANDLE pipe;
public:
	Bootstrap();
	~Bootstrap();
	void Init();
};

