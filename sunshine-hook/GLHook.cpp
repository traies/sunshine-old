#include "stdafx.h"
#include "GLHook.h"


GLHook::GLHook()
{
}

GLHook::~GLHook()
{
}

//	NOTE: IMPORTANT! REMEMBER TO CHANGE ID3D11Device to OpenGL "device" or equivalent (probably context)
std::shared_ptr<Encode::GLEncodePipeline> GLHook::GetEncodePipeline(ID3D11Device * device)
{
	if (encodePipeline == nullptr) {
		encodePipeline = std::make_shared<Encode::GLEncodePipeline>();
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
	auto hMod = GetModuleHandle(TEXT("opengl32.dll"));
	if (hMod == nullptr) {
		hMod = LoadLibrary(TEXT("opengl32.dll"));
		if (hMod == nullptr) {
			LOG(ERROR) << "Load opengl32.dll module failed.";
			return false;
		}
	}
	
	swapBuffers = (SWAP_BUFFERS_FUNC) GetProcAddress(hMod, "wglSwapBuffers");
	if (swapBuffers == nullptr) {
		LOG(ERROR) << "GetProcAddress for wglSwapBuffers failed";
		return false;
	}

	InstallHook("wglSwapBuffers", swapBuffers, HookSwapBuffers);
	return true;
}

static BOOL WINAPI HookSwapBuffers(HDC hdc)
{
	//	Perform backbuffer capture and encoding.
	auto hook = GLHook::GetInstance();
	LOG(INFO) << "Swapping buffers.";
	return hook->GetSwapBuffers()(hdc);
}

SWAP_BUFFERS_FUNC GLHook::GetSwapBuffers()
{
	if (swapBuffers == nullptr) {
		// Should probably throw an exception.
		LOG(ERROR) << "Illegal call: EndScene ptr is null";
		ExitProcess(1);
	}
	return swapBuffers;
}

bool GLHook::Uninstall()
{
	return false;
}
