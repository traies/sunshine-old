#pragma once
#include <memory>
#include "Encoder.h"
#include <d3d9.h>
namespace Encode {
	template <typename T>
	class EncodePipeline
	{
	public:
		EncodePipeline() {};
		virtual ~EncodePipeline() {};
		virtual bool Call(T * frame) = 0;
	};
}


