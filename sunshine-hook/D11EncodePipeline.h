#pragma once
#include "EncodePipeline.h"
#include "AmdEncoder.h"
#include <d3d11.h>
#include <thread>

namespace Encode {
	class D11EncodePipeline: public EncodePipeline<ID3D11Texture2D>
	{
	public:
		D11EncodePipeline(ID3D11Device * device, HANDLE pipe, std::shared_ptr<UDPClient> socket)
			: EncodePipeline(std::make_unique<AmdEncoder>(device), pipe, socket)
		{
		};
		bool Call(ID3D11Texture2D * frame) override;
	private:
		D11EncodePipeline();
	};
}


