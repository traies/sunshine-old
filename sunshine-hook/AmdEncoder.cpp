#include "stdafx.h"
#include "AmdEncoder.h"
#include <include\components\VideoEncoderVCE.h>
#include "..\easyloggingpp\easylogging++.h"
#include <gl\GL.h>

#pragma comment (lib, "AmfMediaCommon.lib")
#pragma comment (lib, "opengl32.lib")

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


void AmdEncoder::Init(IDirect3DDevice9 * device)
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


static bool InitDX11Device(ID3D11Device * ogDevice, ID3D11Device** device)
{
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_9_1
	};
	ID3D11DeviceContext* context;
	D3D_FEATURE_LEVEL featureLevel;
	auto hr = D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		ogDevice->GetCreationFlags(),
		featureLevels,
		ARRAYSIZE(featureLevels),
		D3D11_SDK_VERSION,
		device,
		&featureLevel,
		&context
	);

	return true;
}

void AmdEncoder::Init(ID3D11Device * _device)
{
	//InitDX11Device(_device, &device);
	//device->AddRef();
	device = _device;
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

void AmdEncoder::Init(HDC * hdc)
{
	amf::AMFFactory * factory(nullptr);
	{
		auto res = InitContext(&factory);
		if (res != AMF_OK) {
			LOG(ERROR) << "InitContext failed.";
			ExitProcess(1);
		}
	}
	//	Init OpenGL
	{
		auto wglContext = wglGetCurrentContext();
		if (context == nullptr) {
			LOG(ERROR) << "Failed to get context.";
			ExitProcess(1);
		}
		
		//	
		auto res = context->InitOpenGL(wglContext,nullptr,*hdc);
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

static amf::AMF_SURFACE_FORMAT ConvertD3D11ToAMF(DXGI_FORMAT format)
{
	switch (format) {
	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		return amf::AMF_SURFACE_BGRA;
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		return amf::AMF_SURFACE_RGBA;
	default:
		return amf::AMF_SURFACE_RGBA;
	}
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

	auto format = ConvertD3D11ToAMF(sd.Format);
	{
		auto res = SetEncoderProperties(width, height, framerate, format);
		if (!res) {
			LOG(ERROR) << "Direct3D11 encoder init failed.";
			return false;
		}
	}

	//	Create offscreen surface.
	{
		auto res = context->AllocSurface(amf::AMF_MEMORY_DX11, ConvertD3D11ToAMF(sd.Format), sd.Width, sd.Height, &surfaceamf);
		if (res != AMF_OK) {
			LOG(ERROR) << "CreateSurfaceFromDX11Native failed.";
			return false;
		}
	}

	encoderStarted = true;
	LOG(INFO) << "Encoder started";
	return true;
}

bool AmdEncoder::InitEncoder(GLuint * texture)
{
	int width;
	int height;
	int f;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
	//glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &f);
	int framerate = 60;
	auto format = amf::AMF_SURFACE_RGBA;
	{
		auto res = SetEncoderProperties(width, height, framerate, format);
		if (!res) {
			LOG(ERROR) << "OpenGL encoder init failed.";
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

static void FillSurfaceDX11(amf::AMFContext* context, amf::AMFSurface* surface, ID3D11Texture2D* frame)
{
	HRESULT hr = S_OK;
	// fill surface with something something useful. We fill with color and color rect
	// get native DX objects
	ID3D11Device* deviceDX11 = (ID3D11Device*)context->GetDX11Device(); // no reference counting - do not Release()
	ID3D11Texture2D* surfaceDX11 = (ID3D11Texture2D*)surface->GetPlaneAt(0)->GetNative(); // no reference counting - do not Release()

	ID3D11DeviceContext* deviceContextDX11 = NULL;
	deviceDX11->GetImmediateContext(&deviceContextDX11);
	deviceContextDX11->CopyResource(surfaceDX11, frame);
	deviceContextDX11->Release();
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
	
	FillSurfaceDX11(context, surfaceamf, frame);

	//	Duplicate Buffer and send to encoder.
	auto res = SendSurfaceToEncoder(surfaceamf);
	if (!res) {
		LOG(ERROR) << "DuplicateBuffer failed.";
	}
	return true;
}

bool AmdEncoder::PutFrame(GLuint * texture)
{
	//	Init encoder (but just once).
	if (!encoderStarted) {
		auto res = InitEncoder(texture);
		if (!res) {
			LOG(ERROR) << "Put frame failed.";
			ExitProcess(1);
		}
	}

	//	Create offscreen surface.
	amf::AMFSurfacePtr surfaceamf;
	{
		auto res = context->CreateSurfaceFromOpenGLNative(amf::AMF_SURFACE_RGBA,(amf_handle) *texture, &surfaceamf, nullptr);
		if (res != AMF_OK) {
			LOG(ERROR) << "CreateSurfaceFromOpenGLNative failed.";
			return false;
		}
	}

	//	Submit duplicate backbuffer to encoder.
	{
		auto res = encoder->SubmitInput(surfaceamf);
		if (res != AMF_OK) {
			LOG(ERROR) << "Encoder SubmitInput failed.";
			return false;
		}
	}
	return true;
}

bool AmdEncoder::SendSurfaceToEncoder(amf::AMFSurfacePtr surface)
{
	//	Submit duplicate backbuffer to encoder.
	AMF_RESULT res = encoder->SubmitInput(surface);
	if (res != AMF_OK) {
		LOG(ERROR) << "Encoder SubmitInput failed." << res;
		return false;
	}
	return true;
}

bool AmdEncoder::PullBuffer(std::vector<std::vector<uint8_t>>& buff) 
{
	amf::AMFData * d;
	auto res = encoder->QueryOutput(&d);
	if (res == AMF_EOF) {
		//	No more frames are comming.
		LOG(INFO) << "AMF_EOF: encoder was closed?";
		std::vector<std::vector<uint8_t>> v;
		return false;
	}
	else if (res == AMF_OK) {
		amf::AMFBufferPtr buffer(d);
		auto data = static_cast<uint8_t *>(buffer->GetNative());
		auto size = static_cast<uint64_t>(buffer->GetSize());
		std::vector<uint8_t>d (data, data + size);
		buff.push_back(d);
		return true;
	}
	else {
		Sleep(2);
		return false;
	}
	
}
