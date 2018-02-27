#pragma once
#include "EncodePipeline.h"
#include "AmdEncoder.h"
#include <d3d11.h>

namespace Encode {
	class D11EncodePipeline: public EncodePipeline<ID3D11Texture2D>
	{
	public:
		D11EncodePipeline(ID3D11Device * device, HANDLE pipe) : encoder(std::make_unique<AmdEncoder>(device)), pipe(pipe) {};
		~D11EncodePipeline();
		bool Call(ID3D11Texture2D * frame) override;
	private:
		D11EncodePipeline();
		std::unique_ptr<AmdEncoder> encoder;
		HANDLE pipe;
	};
}


