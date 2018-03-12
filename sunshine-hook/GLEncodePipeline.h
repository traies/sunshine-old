#pragma once
#include "EncodePipeline.h"
#include "AmdEncoder.h"
#include <thread>
namespace Encode {
	class GLEncodePipeline : public EncodePipeline<HDC>
	{
	public:
		GLEncodePipeline(HDC * hdc, HANDLE pipe, std::shared_ptr<UDPClient> socket): 
			EncodePipeline(std::make_unique<AmdEncoder>(hdc), pipe, socket)
		{
		};

		bool Call(HDC * frame) override;
	private:
		GLEncodePipeline();
	};
}


