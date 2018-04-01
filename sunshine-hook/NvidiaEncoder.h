#pragma once
#include "Encoder.h"
#include <d3d9.h>
#include <memory>
#include "..\easyloggingpp\easylogging++.h"
#include <NvEncoder\NvEncoder.h>
#include <NvEncoder\NvEncoderD3D9.h>
#include <queue>

namespace Encode {
	class NvidiaEncoder : public Encoder
	{
	public:
		NvidiaEncoder(IDirect3DDevice9 * device) :
			d3d9device(device) {};
		NvidiaEncoder(ID3D11Device * device):
			d3d11device(device) {};
		NvidiaEncoder(HDC * hdc) :
			hdc(hdc) {};
		~NvidiaEncoder() {};
		bool PutFrame(IDirect3DSurface9 * frame) override;
		bool PutFrame(ID3D11Texture2D * frame) override;
		bool PutFrame(GLuint * texture) override;
		std::unique_ptr<std::vector<std::vector<uint8_t>>> PullBuffer() override;
	private:
		IDirect3DDevice9 * d3d9device;
		ID3D11Device * d3d11device;
		HDC * hdc;
		std::unique_ptr<NvEncoderD3D9> encoder;
		std::queue <std::vector<std::vector<uint8_t>>> queue;
		std::mutex m;
		bool InitEncoder(IDirect3DSurface9 * frame);
	};
}


