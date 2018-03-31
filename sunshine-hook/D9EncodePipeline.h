#pragma once
#include "EncodePipeline.h"
#include <d3d9.h>
#include <thread>
#include <boost\asio.hpp>
#include "UDPClient.h"

namespace Encode {
	template <typename EncType>
	class D9EncodePipeline  : public EncodePipeline<IDirect3DSurface9, EncType>
	{
	public:
		D9EncodePipeline(IDirect3DDevice9 * device, HANDLE pipe, std::shared_ptr<UDPClient> socket) 
			: EncodePipeline<IDirect3DSurface9, EncType>(std::make_unique<EncType>(device), pipe, socket)
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


