#include "stdafx.h"
#include "AmdEncoder.h"
#include <include\components\VideoEncoderVCE.h>
#include "..\easyloggingpp\easylogging++.h"

#pragma comment (lib, "AmfMediaCommon.lib") 

using namespace Encode;

AmdEncoder::AmdEncoder(IDirect3DDevice9 * device)
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
	amf::AMFFactory * factory(nullptr);
	{
		auto res = init(version, &factory);
		if (res != AMF_OK) {
			LOG(ERROR) << "AmfFactory init failed.";
			ExitProcess(1);
		}
	}

	//	Create context
	{
		auto res = factory->CreateContext(&context);
		if (res != AMF_OK) {
			LOG(ERROR) << "CreateContext failed";
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

bool AmdEncoder::InitEncoder(IDirect3DSurface9 * frame) 
{
	//	Get frame description. Probably should save this and compare each frame submitted to monitor changes.
	D3DSURFACE_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	frame->GetDesc(&sd);
	auto width = sd.Width;
	auto height = sd.Height;
	auto framerate = 60;

	//	Properties for low latency encoding.
	encoder->SetProperty(AMF_VIDEO_ENCODER_B_PIC_PATTERN, 0);
	encoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_ULTRA_LOW_LATENCY);
	encoder->SetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, AMF_VIDEO_ENCODER_QUALITY_PRESET_SPEED);
	encoder->SetProperty(AMF_VIDEO_ENCODER_FRAMESIZE, AMFConstructSize(width, height));
	encoder->SetProperty(AMF_VIDEO_ENCODER_FRAMERATE, AMFConstructRate(framerate, 1));
	encoder->Init(amf::AMF_SURFACE_BGRA, width, height);

	//	Do not init the encoder again.
	encoderStarted = true;
	return true;
}


bool AmdEncoder::PutFrame(IDirect3DSurface9 * frame)
{
	//	Init encoder (but just once).
	if (!encoderStarted) {
		InitEncoder(frame);
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
	
	//	Duplicate backbuffer.
	amf::AMFDataPtr pDuplicated;
	{
		auto res = surfaceamf->Duplicate(surfaceamf->GetMemoryType(), &pDuplicated);
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
