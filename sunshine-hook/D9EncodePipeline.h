#pragma once
#include "EncodePipeline.h"
#include "AmdEncoder.h"
#include <d3d9.h>

namespace Encode {
	class D9EncodePipeline : public EncodePipeline<IDirect3DSurface9>
	{
	private:
		D9EncodePipeline();
		std::unique_ptr<AmdEncoder> encoder;
		HANDLE pipe;
	public:
		D9EncodePipeline(IDirect3DDevice9 * device, HANDLE pipe) : encoder(std::make_unique<AmdEncoder>(device)), pipe(pipe) {};
		~D9EncodePipeline();
		bool Call(IDirect3DSurface9 * frame) override;
	};
}


