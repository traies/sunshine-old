#pragma once
#include "GraphicHook.h"
#include "GLEncodePipeline.h"
#include "AmdEncoder.h"

typedef BOOL(WINAPI * SWAP_BUFFERS_FUNC)(
	HDC hdcm);

static BOOL WINAPI HookSwapBuffers(
	HDC hdc);

typedef BOOL(WINAPI * SWAP_LAYER_BUFFERS_FUNC)(
	HDC hdcm,
	UINT fuPlanes);

class GLHook: public GraphicHook<Encode::GLEncodePipeline, HDC>
{
public:
	GLHook(std::unique_ptr<Encode::Encoder> encoder) : GraphicHook(std::move(encoder)) {};
	static GLHook* CreateInstance(std::unique_ptr<Encode::Encoder> encoder);
	
	std::shared_ptr<Encode::GLEncodePipeline> GetEncodePipeline(HDC * device) override;
	
	static GLHook* GetInstance();
	
	bool Install()
	{
		auto hMod = GetModuleHandle(TEXT("Gdi32.dll"));
		if (hMod == nullptr) {
			hMod = LoadLibrary(TEXT("Gdi32.dll"));
			if (hMod == nullptr) {
				LOG(ERROR) << "Load Gdi32.dll module failed.";
				return false;
			}
		}

		swapBuffers = (SWAP_BUFFERS_FUNC)GetProcAddress(hMod, "SwapBuffers");
		if (swapBuffers == nullptr) {
			LOG(ERROR) << "GetProcAddress for SwapBuffers failed";
			return false;
		}
		return this->InstallHook("SwapBuffers", swapBuffers, HookSwapBuffers);
	};
	bool Uninstall();
	SWAP_BUFFERS_FUNC GetSwapBuffers();
private:
	static GLHook* instance;
	std::shared_ptr<Encode::GLEncodePipeline> encodePipeline;
	SWAP_BUFFERS_FUNC swapBuffers;
};

static BOOL WINAPI HookSwapBuffers(HDC hdc)
{
	//	Perform backbuffer capture and encoding.
	auto hook = GLHook::GetInstance();

	auto encodePipeline = hook->GetEncodePipeline(&hdc);

	auto res = encodePipeline->Call(&hdc);
	LOG(INFO) << "Swapping buffers.";
	return hook->GetSwapBuffers()(hdc);
}
