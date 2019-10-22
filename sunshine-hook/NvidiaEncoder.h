#pragma once
#include "UDPClient.h"
#include "Encoder.h"
#include <d3d9.h>
#include <memory>
#include "..\easyloggingpp\easylogging++.h"
#include <NvEncoder\NvEncoder.h>
#include <NvEncoder\NvEncoderD3D9.h>
#include <NvEncoder\NvEncoderD3D11.h>
#include <queue>
#include <mutex>

namespace Encode {
	class NvidiaEncoder : public Encoder
	{
	public:

		void Init(IDirect3DDevice9* device) override
		{
			d3d9device = device;
		};
		void Init(ID3D11Device* device) override 
		{
			d3d11device = device;
		};
		void Init(HDC* hdc) override
		{
			hdc = hdc;
		};
		~NvidiaEncoder() 
		{
			try {
				if (encoder != nullptr) {
					std::vector<std::vector<uint8_t>> vPacket;
					encoder->EndEncode(vPacket);
					encoder.release();
				}
			}
			catch (std::exception& ex) {
				LOG(ERROR) << ex.what();
			}
		};
		bool PutFrame(IDirect3DSurface9 * frame) override;
		bool PutFrame(ID3D11Texture2D * frame) override;
		bool PutFrame(GLuint * texture) override;
		std::vector<std::vector<uint8_t>> PullBuffer() override;
	private:
		IDirect3DDevice9 * d3d9device = nullptr;
		ID3D11Device * d3d11device = nullptr;
		HDC * hdc = nullptr;
		std::unique_ptr<NvEncoder> encoder;
		std::queue <std::vector<std::vector<uint8_t>>> queue;
		std::queue <ID3D11Texture2D*> textureQueue;
		bool InitEncoder(IDirect3DSurface9 * frame);
		bool InitEncoder(ID3D11Texture2D * frame);
		bool encoderInit = false;
		bool f = false;
		int frames = 0;
		std::mutex frameLock;
	};
}


