#include "stdafx.h"
#include "GLHook.h"
#include "AmdEncoder.h"

std::shared_ptr<Encode::GLEncodePipeline> GLHook::GetEncodePipeline(HDC * hdc)
{
	if (encodePipeline == nullptr) {
		_encoder->Init(hdc);
		encodePipeline = std::make_shared<Encode::GLEncodePipeline>(std::move(_encoder), this->pipe, this->socket);
	}
	return encodePipeline;

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

GLHook* GLHook::GetInstance()
{
	if (instance == nullptr) {
		LOG(ERROR) << "Tried to get GL hook without setting it up first.";
	}
	return instance;
};

GLHook* GLHook::CreateInstance(std::unique_ptr<Encode::Encoder> encoder)
{
	instance = new GLHook(std::move(encoder));
	return instance;
}

GLHook* GLHook::instance;