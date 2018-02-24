#pragma once
#include "Hook.h"
#include "D9Hook.h"
class Bootstrap
{
private:
	void InitLogger();
	void InstallHookD9();
	void InstallHookD11();
public:
	Bootstrap();
	~Bootstrap();
	void Init();
};

