#pragma once
#include "Encoder.h"
#include <d3d9.h>
#include <d3d11.h>
#include <include\core\Factory.h>

namespace Encode {
	class AmdEncoder : public Encoder<amf::AMFData>
	{
	public:
		AmdEncoder(IDirect3DDevice9 * device);
		AmdEncoder(ID3D11Device * device);
		~AmdEncoder();
		bool PutFrame(IDirect3DSurface9 * frame) override;
		bool PutFrame(ID3D11Texture2D * frame) override;
		bool PullBuffer(amf::AMFData ** buffer) override;
	private:
		AmdEncoder() {};
		amf::AMFContextPtr context;
		amf::AMFComponentPtr encoder;
		bool InitContext(amf::AMFFactory ** factory);
		bool InitEncoder(IDirect3DSurface9 * frame);
		bool InitEncoder(ID3D11Texture2D * texture);
		bool SetEncoderProperties(UINT width, UINT height, UINT frame, amf::AMF_SURFACE_FORMAT f);
		bool SendSurfaceToEncoder(amf::AMFSurfacePtr surface);
		bool encoderStarted = false;
	};

}

