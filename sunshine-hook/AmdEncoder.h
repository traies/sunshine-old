#pragma once
#include "Encoder.h"
#include <d3d9.h>
#include <include\core\Factory.h>

namespace Encode {
	class AmdEncoder : public Encoder<IDirect3DSurface9, amf::AMFData>
	{
	public:
		
		AmdEncoder(IDirect3DDevice9 * device);
		~AmdEncoder();
		bool PutFrame(IDirect3DSurface9 * frame) override;
		bool PullBuffer(amf::AMFData ** buffer) override;
	private:
		AmdEncoder() {};
		amf::AMFContextPtr context;
		amf::AMFComponentPtr encoder;
		bool InitEncoder(IDirect3DSurface9 * frame);
		bool encoderStarted = false;
	};

}

