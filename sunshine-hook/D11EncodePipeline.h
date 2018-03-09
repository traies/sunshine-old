#pragma once
#include "EncodePipeline.h"
#include "AmdEncoder.h"
#include <d3d11.h>
#include <thread>

namespace Encode {
	class D11EncodePipeline: public EncodePipeline<ID3D11Texture2D>
	{
	public:
		D11EncodePipeline(ID3D11Device * device, HANDLE pipe) : encoder(std::make_unique<AmdEncoder>(device)), pipe(pipe)
		{
			encodeThread = std::thread(&D11EncodePipeline::Encode, this);
		};
		~D11EncodePipeline();
		bool Call(ID3D11Texture2D * frame) override;
	private:
		D11EncodePipeline();
		void Encode();
		std::thread encodeThread;
		std::unique_ptr<AmdEncoder> encoder;
		HANDLE pipe;
	};
}


