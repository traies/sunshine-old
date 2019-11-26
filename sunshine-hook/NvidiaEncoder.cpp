#include "stdafx.h"
#include "NvidiaEncoder.h"
#include <d3d9.h>
#include <D3dx9tex.h>
#include <utility>
#include <d3d11.h>

#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "d3d11.lib")
using namespace Encode;

NvidiaEncoder::NvidiaEncoder(const RemoteProcessStartInfo& startup)
{
	this->bitrate = startup.bitrate;
	
	if (startup.codec == CODEC::H264) {
		codec = NV_ENC_CODEC_H264_GUID;
	}

	if (startup.codec == CODEC::HEVC) {
		codec = NV_ENC_CODEC_HEVC_GUID;
	}
};

bool NvidiaEncoder::PutFrame(IDirect3DSurface9 * frame)
{
	if (encoder == nullptr) {
		if (d3d9device == nullptr) {
			throw std::exception("D3D9Device is not initialized");
		}
		if (!InitEncoder(frame)) {
			return true;
		}
	}
	if (queue.size() < 2) {
		frameLock.lock();
		auto surf = reinterpret_cast<IDirect3DSurface9 *>(encoder->GetNextInputFrame()->inputPtr);
		D3DXLoadSurfaceFromSurface(surf, nullptr, nullptr, frame, nullptr, nullptr, D3DX_DEFAULT, 0);
		std::vector<std::vector<uint8_t>> vPacket;
		encoder->EncodeFrame(vPacket);
		queue.push(vPacket);
		frameLock.unlock();
	}
	return true;
}

int frameCount = 0;
ID3D11Texture2D* texture;

bool NvidiaEncoder::PutFrame(ID3D11Texture2D * frame)
{
	if (encoder == nullptr) {
		if (d3d11device == nullptr) {
			throw std::exception("D3D11Device is not initialized");
		}
		if (!InitEncoder(frame)) {
			return true;
		}
	}


	if (queue.size() < 2) {
		frameLock.lock();
		
		ID3D11Device* device;
		frame->GetDevice(&device);

		ID3D11DeviceContext* context;
		device->GetImmediateContext(&context);
		context->CopyResource(auxFrame, frame);
		std::vector<std::vector<uint8_t>> buffer;
		queue.push(buffer);
		frameLock.unlock();
	}
	return true;
}

bool NvidiaEncoder::PutFrame(GLuint * texture)
{
	return false; 
}

bool NvidiaEncoder::PullBuffer(std::vector<std::vector<uint8_t>>& buffer)
{
	std::lock_guard<std::mutex> lock(frameLock);
	if (!queue.empty()) {
		frameCount++;
		if (frameCount > 0 && frameCount % 300 == 0) {
			NV_ENC_RECONFIGURE_PARAMS params = { NV_ENC_RECONFIGURE_PARAMS_VER };
			memcpy(&params.reInitEncodeParams, &initializeParams, sizeof(initializeParams));
			NV_ENC_CONFIG reInitCodecConfig = { NV_ENC_CONFIG_VER };
			memcpy(&reInitCodecConfig, initializeParams.encodeConfig, sizeof(reInitCodecConfig));
			params.reInitEncodeParams.encodeConfig = &reInitCodecConfig;
			params.forceIDR = true;
			encoder->Reconfigure(&params);
		}

		buffer = queue.front();
		ID3D11DeviceContext* context;
		d3d11device->GetImmediateContext(&context);
		auto surf = reinterpret_cast<ID3D11Texture2D*>(encoder->GetNextInputFrame()->inputPtr);
		context->CopyResource(surf, auxFrame);

		//context->Flush();
		encoder->EncodeFrame(buffer);

		queue.pop();
		//texture->;
		return true;
	}
	return false;
}

bool NvidiaEncoder::InitEncoder(IDirect3DSurface9 * frame)
{
	D3DSURFACE_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	frame->GetDesc(&sd);
	int width = sd.Width;
	int height = sd.Height;
	auto f = sd.Format;
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
	encoder->CreateDefaultEncoderParams(&initializeParams, this->codec, NV_ENC_PRESET_LOW_LATENCY_HP_GUID);
	encodeConfig.rcParams.rateControlMode = NV_ENC_PARAMS_RC_CBR;
	encodeConfig.rcParams.averageBitRate =this->bitrate;
	encodeConfig.rcParams.vbvBufferSize = (encodeConfig.rcParams.averageBitRate * initializeParams.frameRateDen / initializeParams.frameRateNum) * 5;
	encodeConfig.rcParams.maxBitRate = encodeConfig.rcParams.averageBitRate;
	encodeConfig.rcParams.vbvInitialDelay = encodeConfig.rcParams.vbvBufferSize;
	try {
		encoder->CreateEncoder(&initializeParams);
	}
	catch (std::exception& ex) {
		LOG(ERROR) << ex.what();
	}
	
	return true;
}

bool NvidiaEncoder::InitEncoder(ID3D11Texture2D * frame)
{
	D3D11_TEXTURE2D_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	frame->GetDesc(&sd);
	auto width = sd.Width;
	auto height = sd.Height;
	auto d3df = sd.Format;
	NV_ENC_BUFFER_FORMAT format = NV_ENC_BUFFER_FORMAT_ARGB;

	encoder = std::make_unique<NvEncoderD3D11>(
		d3d11device,
		width,
		height,
		format,
		d3df,
		0,
		false);


	ZeroMemory(&initializeParams, sizeof(initializeParams));

	NV_ENC_CONFIG * encodeConfig = new NV_ENC_CONFIG{ NV_ENC_CONFIG_VER };
	initializeParams.encodeConfig = encodeConfig;

	encoder->CreateDefaultEncoderParams(&initializeParams, this->codec, NV_ENC_PRESET_LOW_LATENCY_HP_GUID);
	encodeConfig->rcParams.averageBitRate = this->bitrate;
	encodeConfig->rcParams.vbvBufferSize = 0; 
	encodeConfig->rcParams.vbvInitialDelay = 0;
	encodeConfig->rcParams.maxBitRate = encodeConfig->rcParams.averageBitRate;
	encodeConfig->rcParams.rateControlMode = NV_ENC_PARAMS_RC_CBR;
	try {
		encoder->CreateEncoder(&initializeParams);
	}
	catch (std::exception& ex) {
		LOG(ERROR) << ex.what();
	}

	D3D11_TEXTURE2D_DESC destDesc;
	ZeroMemory(&destDesc, sizeof(destDesc));
	destDesc.Format = sd.Format;
	destDesc.Height = sd.Height;
	destDesc.Width = sd.Width;
	destDesc.Usage = D3D11_USAGE_DEFAULT;
	destDesc.CPUAccessFlags = 0;
	destDesc.BindFlags = 0;
	destDesc.MipLevels = destDesc.ArraySize = 1;
	destDesc.SampleDesc.Quality = 0;
	destDesc.SampleDesc.Count = 1;
	destDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

	auto res = d3d11device->CreateTexture2D(&destDesc, nullptr, &auxFrame);

	return true;
}