#pragma once
#include "EncodePipeline.h"
#include "AmdEncoder.h"
#include <d3d9.h>
#include <thread>
#include <boost\asio.hpp>
#include "UDPClient.h"

namespace Encode {
	class D9EncodePipeline : public EncodePipeline<IDirect3DSurface9>
	{
	private:
		D9EncodePipeline();
		
		
	public:
		D9EncodePipeline(IDirect3DDevice9 * device, HANDLE pipe, std::shared_ptr<UDPClient> socket) 
			: EncodePipeline(std::make_unique<AmdEncoder>(device), pipe, socket)
		{
		};
		bool Call(IDirect3DSurface9 * frame) override;
	};
}


