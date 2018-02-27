#include "stdafx.h"
#include "AmdEncoder.h"
#include <include\components\VideoEncoderVCE.h>
#include "..\easyloggingpp\easylogging++.h"

#pragma comment (lib, "AmfMediaCommon.lib") 

using namespace Encode;

bool AmdEncoder::InitContext(amf::AMFFactory ** factory)
{
	HMODULE hAmfDll = LoadLibrary(AMF_DLL_NAME);
	if (hAmfDll == nullptr) {
		LOG(ERROR) << "Amf LoadLibrary failed.";
		ExitProcess(1);
	}
	//	Query AMF version, just for development clarity.
	AMFQueryVersion_Fn queryVersion = (AMFQueryVersion_Fn)GetProcAddress(hAmfDll, AMF_QUERY_VERSION_FUNCTION_NAME);
	uint64_t version = 0;
	{
		auto res = queryVersion(&version);
		if (res != AMF_OK) {
			LOG(ERROR) << "Amf QueryVersion failed.";
			ExitProcess(1);
		}
	}
	LOG(INFO) << "Starting AMF, version " << version << ".";

	AMFInit_Fn init = (AMFInit_Fn)GetProcAddress(hAmfDll, AMF_INIT_FUNCTION_NAME);

	//	Start AMF factory
	{
		auto res = init(version, factory);
		if (res != AMF_OK) {
			LOG(ERROR) << "AmfFactory init failed.";
			ExitProcess(1);
		}
	}

	//	Create context
	{
		auto res = (*factory)->CreateContext(&context);
		if (res != AMF_OK) {
			LOG(ERROR) << "CreateContext failed";
			return false;
		}
	}
}


AmdEncoder::AmdEncoder(IDirect3DDevice9 * device)
{
	amf::AMFFactory * factory(nullptr);
	{
		auto res = InitContext(&factory);
		if (res != AMF_OK) {
			LOG(ERROR) << "InitContext failed.";
			ExitProcess(1);
		}
	}

	//	Now, InitDX9 should be called.
	{
		auto res = context->InitDX9(device);
		if (res != AMF_OK) {
			LOG(ERROR) << "InitDX9 failed";
			ExitProcess(1);
		}
	}

	//	Create encoder. However, we need width, height and 
	//	format of the backbuffer before we call encoder->init.
	{
		auto res = factory->CreateComponent(context, AMFVideoEncoderVCE_AVC, &encoder);
		if (res != AMF_OK) {
			LOG(ERROR) << "CreateComponent failed.";
			ExitProcess(1);
		}
	}

	LOG(INFO) << "Everything was created fine.";
}

AmdEncoder::AmdEncoder(ID3D11Device * device)
{
	amf::AMFFactory * factory(nullptr);
	{
		auto res = InitContext(&factory);
		if (res != AMF_OK) {
			LOG(ERROR) << "InitContext failed.";
			ExitProcess(1);
		}
	}

	//	Now, InitDX11 should be called.
	{
		auto res = context->InitDX11(device);
		if (res != AMF_OK) {
			LOG(ERROR) << "InitDX11 failed";
			ExitProcess(1);
		}
	}

	//	Create encoder. However, we need width, height and 
	//	format of the backbuffer before we call encoder->init.
	{
		auto res = factory->CreateComponent(context, AMFVideoEncoderVCE_AVC, &encoder);
		if (res != AMF_OK) {
			LOG(ERROR) << "CreateComponent failed.";
			ExitProcess(1);
		}
	}

	LOG(INFO) << "Everything was created fine.";
}

bool AmdEncoder::SetEncoderProperties(UINT width, UINT height, UINT framerate, amf::AMF_SURFACE_FORMAT format)
{
	if (encoder == nullptr) {
		LOG(ERROR) << "Encoder not initialized";
		return false;
	}

	//	Properties for low latency encoding.
	encoder->SetProperty(AMF_VIDEO_ENCODER_B_PIC_PATTERN, 0);
	encoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_ULTRA_LOW_LATENCY);
	encoder->SetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, AMF_VIDEO_ENCODER_QUALITY_PRESET_SPEED);
	encoder->SetProperty(AMF_VIDEO_ENCODER_FRAMESIZE, AMFConstructSize(width, height));
	encoder->SetProperty(AMF_VIDEO_ENCODER_FRAMERATE, AMFConstructRate(framerate, 1));

	auto res = encoder->Init(format, width, height);

	if (res != AMF_OK) {
		LOG(ERROR) << "Encoder Init failed.";
		return false;
	}
	return true;
}

bool AmdEncoder::InitEncoder(IDirect3DSurface9 * frame) 
{
	//	Get frame description. Probably should save this and compare each frame submitted to monitor changes.
	D3DSURFACE_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	frame->GetDesc(&sd);
	auto width = sd.Width;
	auto height = sd.Height;
	auto framerate = 60;
	auto format = amf::AMF_SURFACE_BGRA;

	{
		auto res = SetEncoderProperties(width, height, framerate, format);
		if (!res) {
			LOG(ERROR) << "Direct3D9 encoder init failed.";
			return false;
		}
	}
	//	Do not init the encoder again.
	encoderStarted = true;
	return true;
}

bool AmdEncoder::InitEncoder(ID3D11Texture2D * frame)
{
	D3D11_TEXTURE2D_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	frame->GetDesc(&sd);
	auto width = sd.Width;
	auto height = sd.Height;
	auto framerate = 60;

	//	On testing, all Directx11 games work with this format, while Directx9 games work with AMF_SURFACE_BGRA.
	//	Better format selection should be done in the future.
	auto format = amf::AMF_SURFACE_RGBA;
	{
		auto res = SetEncoderProperties(width, height, framerate, format);
		if (!res) {
			LOG(ERROR) << "Direct3D11 encoder init failed.";
			return false;
		}
	}
	encoderStarted = true;
	return true;
}

bool AmdEncoder::PutFrame(IDirect3DSurface9 * frame)
{
	//	Init encoder (but just once).
	if (!encoderStarted) {
		auto res = InitEncoder(frame);
		if (!res) {
			LOG(ERROR) << "Put frame failed.";
			ExitProcess(1);
		}
	}

	//	Create offscreen surface.
	amf::AMFSurfacePtr surfaceamf;
	{
		auto res = context->CreateSurfaceFromDX9Native(frame, &surfaceamf, nullptr);
		if (res != AMF_OK) {
			LOG(ERROR) << "CreateSurfaceFromDX9Native failed.";
			return false;
		}
	}
	
	//	Duplicate Buffer and send to encoder.
	{
		auto res = SendSurfaceToEncoder(surfaceamf);
		if (!res) {
			LOG(ERROR) << "DuplicateBuffer failed.";
			return false;
		}
	}

	return true;
}

bool AmdEncoder::PutFrame(ID3D11Texture2D * frame)
{
	//	Init encoder (but just once).
	if (!encoderStarted) {
		auto res = InitEncoder(frame);
		if (!res) {
			LOG(ERROR) << "Put frame failed.";
			ExitProcess(1);
		}
	}

	//	Create offscreen surface.
	amf::AMFSurfacePtr surfaceamf;
	{
		auto res = context->CreateSurfaceFromDX11Native(frame, &surfaceamf, nullptr);
		if (res != AMF_OK) {
			LOG(ERROR) << "CreateSurfaceFromDX11Native failed.";
			return false;
		}
	}

	//	Duplicate Buffer and send to encoder.
	{
		auto res = SendSurfaceToEncoder(surfaceamf);
		if (!res) {
			LOG(ERROR) << "DuplicateBuffer failed.";
			return false;
		}
	}
	return true;
}

bool AmdEncoder::SendSurfaceToEncoder(amf::AMFSurfacePtr surface)
{
	//	Duplicate backbuffer.
	amf::AMFDataPtr pDuplicated(nullptr);
	{
		auto res = surface->Duplicate(surface->GetMemoryType(), &pDuplicated);
		if (res != AMF_OK) {
			LOG(ERROR) << "Duplicate failed.";
			return false;
		}
	}

	//	Submit duplicate backbuffer to encoder.
	{
		auto res = encoder->SubmitInput(pDuplicated);
		if (res != AMF_OK) {
			LOG(ERROR) << "Encoder SubmitInput failed.";
			return false;
		}
	}
	return true;
}

bool AmdEncoder::PullBuffer(amf::AMFData ** data)
{
	amf::AMFData * d;
	auto res = encoder->QueryOutput(&d);
	if (res == AMF_EOF) {
		//	No more frames are comming.
		LOG(INFO) << "AMF_EOF: encoder was closed?";
		return false;
	} 
	*data = d;
	return true;
}
AmdEncoder::~AmdEncoder()
{
}
