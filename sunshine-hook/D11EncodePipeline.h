#pragma once
#include "EncodePipeline.h"
#include <d3d11.h>
#include <thread>

namespace Encode {
	class D11EncodePipeline: public EncodePipeline<ID3D11Texture2D>
	{
	public:
		D11EncodePipeline(std::unique_ptr<Encoder> encoder, HANDLE pipe, std::shared_ptr<UDPClientNew> socket)
			: EncodePipeline<ID3D11Texture2D>(std::move(encoder), pipe, socket)
		{
		};
		bool Call(ID3D11Texture2D * frame) override
		{
			//	Submit frame to encoder.
			if (!this->encoder->PutFrame(frame)) {
				LOG(ERROR) << "Frame encoding failed.";
				return false;
			}
			return true;
		}
	};
}


