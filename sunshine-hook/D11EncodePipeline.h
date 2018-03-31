#pragma once
#include "EncodePipeline.h"
#include <d3d11.h>
#include <thread>

namespace Encode {
	template <typename EncType>
	class D11EncodePipeline: public EncodePipeline<ID3D11Texture2D, EncType>
	{
	public:
		D11EncodePipeline(ID3D11Device * device, HANDLE pipe, std::shared_ptr<UDPClient> socket)
			: EncodePipeline<ID3D11Texture2D, EncType>(std::make_unique<EncType>(device), pipe, socket)
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


