#include "stdafx.h"
#include "NvidiaEncoder.h"
#include <d3d9.h>
#include <D3dx9tex.h>
#include <utility>


#pragma comment(lib, "d3dx9.lib")

using namespace Encode;

bool NvidiaEncoder::PutFrame(IDirect3DSurface9 * frame)
{
	if (encoder == nullptr) {
		if (d3d9device == nullptr) {
			throw std::exception("D3D9Device is not initialized");
		}
		InitEncoder(frame);
	}
	auto surf = reinterpret_cast<IDirect3DSurface9 *>(encoder->GetNextInputFrame()->inputPtr);
	D3DXLoadSurfaceFromSurface(surf, nullptr, nullptr, frame, nullptr, nullptr, D3DX_DEFAULT, 0);
	std::vector<std::vector<uint8_t>> vPacket;
	encoder->EncodeFrame(vPacket);
	queue.push(vPacket);
	return true;
}


bool NvidiaEncoder::PutFrame(ID3D11Texture2D * frame)
{
	return false;
}

bool NvidiaEncoder::PutFrame(GLuint * texture)
{
	return false;
}

 auto NvidiaEncoder::PullBuffer() -> std::unique_ptr<std::vector<std::vector<uint8_t>>>
{
	 std::unique_ptr<std::vector<std::vector<uint8_t>>> p = nullptr;
	 m.lock();
	if (!queue.empty()) {
		auto packet = queue.front();
		queue.pop();
		p = std::make_unique<std::vector<std::vector<uint8_t>>>(packet);
	}
	m.unlock();
	return p;
}

bool NvidiaEncoder::InitEncoder(IDirect3DSurface9 * frame)
{
	D3DSURFACE_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	frame->GetDesc(&sd);
	int width = sd.Width;
	int height = sd.Height;
	NV_ENC_BUFFER_FORMAT format = NV_ENC_BUFFER_FORMAT_ARGB;
	encoder = std::make_unique<NvEncoderD3D9>(
		d3d9device,
		width,
		height,
		format,
		nullptr,
		0,
		false);
	NV_ENC_INITIALIZE_PARAMS initializeParams;
	ZeroMemory(&initializeParams, sizeof(initializeParams));
	
	NV_ENC_CONFIG encodeConfig = { NV_ENC_CONFIG_VER };
	initializeParams.encodeConfig = &encodeConfig;
	encoder->CreateDefaultEncoderParams(&initializeParams, NV_ENC_CODEC_H264_GUID, NV_ENC_PRESET_LOW_LATENCY_HP_GUID);
	encodeConfig.rcParams.rateControlMode = NV_ENC_PARAMS_RC_CONSTQP;
	encodeConfig.rcParams.averageBitRate = (static_cast<unsigned int>(5.0f * initializeParams.encodeWidth * initializeParams.encodeHeight) / (1280 * 720)) * 100000;
	encodeConfig.rcParams.vbvBufferSize = (encodeConfig.rcParams.averageBitRate * initializeParams.frameRateDen / initializeParams.frameRateNum) * 5;
	encodeConfig.rcParams.maxBitRate = encodeConfig.rcParams.averageBitRate;
	encodeConfig.rcParams.vbvInitialDelay = encodeConfig.rcParams.vbvBufferSize;
	encoder->CreateEncoder(&initializeParams);
	
	return true;
}
