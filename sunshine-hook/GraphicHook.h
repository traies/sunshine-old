#pragma once
#include "Hook.h"

template <typename PipelineType, typename DeviceType>
class GraphicHook : public Hook
{
public:
	virtual ~GraphicHook() {} ;
	virtual std::shared_ptr<PipelineType> GetEncodePipeline(DeviceType * device) = 0;
	void SetSocket(std::shared_ptr<UDPClient> s)
	{
		socket = s;
	}

protected:
	GraphicHook() {};
	std::shared_ptr<UDPClient> socket;
};
