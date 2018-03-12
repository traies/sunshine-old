#pragma once
#include "Hook.h"
#include "GLEncodePipeline.h"

typedef BOOL(WINAPI * SWAP_BUFFERS_FUNC)(
	HDC hdcm);

static BOOL WINAPI HookSwapBuffers(
	HDC hdc);

typedef BOOL(WINAPI * SWAP_LAYER_BUFFERS_FUNC)(
	HDC hdcm,
	UINT fuPlanes);


static BOOL WINAPI HookSwapLayerBuffers(
	HDC hdc,
	UINT fuPlanes);

class GLHook: public Hook<Encode::GLEncodePipeline, HDC >
{
public:
	GLHook();
	std::shared_ptr<Encode::GLEncodePipeline> GetEncodePipeline(HDC * device) override;
	static std::shared_ptr<GLHook> GetInstance();
	bool Install();
	bool Uninstall();
	SWAP_BUFFERS_FUNC GetSwapBuffers();
	SWAP_LAYER_BUFFERS_FUNC GetSwapLayerBuffers();
private:
	static std::shared_ptr<GLHook> instance;
	std::shared_ptr<Encode::GLEncodePipeline> encodePipeline;
	SWAP_BUFFERS_FUNC swapBuffers;
	SWAP_LAYER_BUFFERS_FUNC swapLayerBuffers;
};

