#pragma once
#include "Encoder.h"
#include <d3d9.h>
#include <d3d11.h>
#include <include\core\Factory.h>

namespace Encode {
	class AmdEncoder : public Encoder {
	public:
		void Init(IDirect3DDevice9 * device) override;
		void Init(ID3D11Device * device) override;
		void Init(HDC * hdc) override;
		~AmdEncoder() {};
		bool PutFrame(IDirect3DSurface9 * frame) override;
		bool PutFrame(ID3D11Texture2D * frame) override;
		bool PutFrame(GLuint * texture) override;
		bool PullBuffer(std::vector<std::vector<uint8_t>>& buff) override;
	private:
		amf::AMFContextPtr context;
		amf::AMFComponentPtr encoder;
		bool InitContext(amf::AMFFactory ** factory);
		bool InitEncoder(IDirect3DSurface9 * frame);
		bool InitEncoder(ID3D11Texture2D * texture);
		bool InitEncoder(GLuint * texture);
		bool SetEncoderProperties(UINT width, UINT height, UINT frame, amf::AMF_SURFACE_FORMAT f);
		bool SendSurfaceToEncoder(amf::AMFSurfacePtr surface);
		bool encoderStarted = false;
	};

}

