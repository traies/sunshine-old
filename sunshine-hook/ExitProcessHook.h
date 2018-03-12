#pragma once
#include "Hook.h"
class ExitProcessHook: public Hook<void, void>
{
public:
	ExitProcessHook();
	~ExitProcessHook();
};

