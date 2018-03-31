#include "stdafx.h"
#include "GLHook.h"
#include "AmdEncoder.h"

template <typename EncType>
std::shared_ptr<Encode::GLEncodePipeline<EncType>> GLHook<EncType>::GetEncodePipeline(HDC * hdc)
{
	if (encodePipeline == nullptr) {
		encodePipeline = std::make_shared<Encode::GLEncodePipeline<EncType>>(hdc, this->pipe, this->socket);
	}
	return encodePipeline;

}

template <typename EncType>
SWAP_BUFFERS_FUNC GLHook<EncType>::GetSwapBuffers()
{
	if (swapBuffers == nullptr) {
		// Should probably throw an exception.
		LOG(ERROR) << "Illegal call: swapBuffers ptr is null";
		ExitProcess(1);
	}
	return swapBuffers;
}

template <typename EncType>
bool GLHook<EncType>::Uninstall()
{
	return false;
}

template <typename EncType>
std::shared_ptr<GLHook<EncType>> GLHook<EncType>::instance;

template <typename EncType>
std::shared_ptr<GLHook<EncType>> GLHook<EncType>::GetInstance()
{
	if (instance == nullptr) {
		instance = std::make_shared<GLHook<EncType>>();
	}
	return instance;
};

template class GLHook<Encode::AmdEncoder>;
