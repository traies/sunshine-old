#include "stdafx.h"
#include "GLHook.h"


GLHook::GLHook()
{
}

GLHook::~GLHook()
{
}

//	NOTE: IMPORTANT! REMEMBER TO CHANGE ID3D11Device to OpenGL "device" or equivalent (probably context)
std::shared_ptr<Encode::GLEncodePipeline> GLHook::GetEncodePipeline(HDC * hdc)
{
	if (encodePipeline == nullptr) {
		encodePipeline = std::make_shared<Encode::GLEncodePipeline>(hdc, pipe);
	}
	return encodePipeline;

}

std::shared_ptr<GLHook> GLHook::instance;

std::shared_ptr<GLHook> GLHook::GetInstance()
{
	if (instance == nullptr) {
		instance = std::make_shared<GLHook>();
	}
	return instance;
}

bool GLHook::Install()
{
	auto hMod = GetModuleHandle(TEXT("Gdi32.dll"));
	if (hMod == nullptr) {
		hMod = LoadLibrary(TEXT("Gdi32.dll"));
		if (hMod == nullptr) {
			LOG(ERROR) << "Load Gdi32.dll module failed.";
			return false;
		}
	}
	
	swapBuffers = (SWAP_BUFFERS_FUNC) GetProcAddress(hMod, "SwapBuffers");
	if (swapBuffers == nullptr) {
		LOG(ERROR) << "GetProcAddress for SwapBuffers failed";
		return false;
	}
	InstallHook("SwapBuffers", swapBuffers, HookSwapBuffers);
	return true;
}

static BOOL WINAPI HookSwapBuffers(HDC hdc)
{
	//	Perform backbuffer capture and encoding.
	auto hook = GLHook::GetInstance();
	
	auto encodePipeline = hook->GetEncodePipeline(&hdc);

	auto res =  encodePipeline->Call(&hdc);
	LOG(INFO) << "Swapping buffers.";
	return hook->GetSwapBuffers()(hdc);
}


SWAP_BUFFERS_FUNC GLHook::GetSwapBuffers()
{
	if (swapBuffers == nullptr) {
		// Should probably throw an exception.
		LOG(ERROR) << "Illegal call: swapBuffers ptr is null";
		ExitProcess(1);
	}
	return swapBuffers;
}

bool GLHook::Uninstall()
{
	return false;
}
