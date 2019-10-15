#pragma once
#include "EncodePipeline.h"
#include <d3d9.h>
#include <thread>
#include <boost\asio.hpp>
#include "UDPClient.h"

namespace Encode {
	class D9EncodePipeline  : public EncodePipeline<IDirect3DSurface9>
	{
	public:
		D9EncodePipeline(std::unique_ptr<Encoder> encoder, HANDLE pipe, std::shared_ptr<UDPClient> socket) 
			: EncodePipeline<IDirect3DSurface9>(std::move(encoder), pipe, socket)
		{
		};
		bool Call(IDirect3DSurface9 * frame) override
		{
			//	Submit frame to encoder.
			if (!this->encoder->PutFrame(frame)) {
				LOG(ERROR) << "Frame encoding failed.";
				return true;
			}

			return true;
		}
	};
}


