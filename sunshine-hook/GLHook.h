#pragma once
#include "Hook.h"
#include "GLEncodePipeline.h"

typedef BOOL(WINAPI * SWAP_BUFFERS_FUNC)(
	HDC hdcm);

static BOOL WINAPI HookSwapBuffers(
	HDC hdc);

class GLHook: public Hook<Encode::GLEncodePipeline, ID3D11Device>
{
public:
	GLHook();
	~GLHook();
	std::shared_ptr<Encode::GLEncodePipeline> GetEncodePipeline(ID3D11Device * device) override;
	static std::shared_ptr<GLHook> GetInstance();
	bool Install();
	bool Uninstall();
	SWAP_BUFFERS_FUNC GetSwapBuffers();
private:
	static std::shared_ptr<GLHook> instance;
	std::shared_ptr<Encode::GLEncodePipeline> encodePipeline;
	SWAP_BUFFERS_FUNC swapBuffers;
};

