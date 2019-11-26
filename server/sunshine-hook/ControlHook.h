#pragma once
#include "Hook.h"
class ControlHook :
	public Hook
{
public:
	bool Install() override;
	bool Uninstall() override
	{
		//TODO correct
		return true;
	}
};

