#include "stdafx.h"
#include "D9Hook.h"
#include <d3d9.h>
#include "AmdEncoder.h"
#include "NvidiaEncoder.h"

#pragma comment (lib, "d3d9.lib")

static HRESULT WINAPI HookEndScene(LPDIRECT3DDEVICE9 device)
{
	//	Perform backbuffer capture and encoding.
	auto hook = D9Hook<Encode::NvidiaEncoder>::GetInstance();
	auto pipeline = hook->GetEncodePipeline(device);

	//	Get back buffer
	IDirect3DSwapChain9 * chain;
	device->GetSwapChain(0, &chain);
	IDirect3DSurface9 * backbuffer;
	chain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);
	if (!pipeline->Call(backbuffer)) {
		LOG(ERROR) << "EncodePipeline failed.";
		//ExitProcess(1);
	}
	return hook->GetEndScene()(device);
}

template<typename EncType>
bool D9Hook<EncType>::Uninstall()
{
	return false;
}

template <typename EncType>
std::shared_ptr<Encode::D9EncodePipeline<EncType>> D9Hook<EncType>::GetEncodePipeline(IDirect3DDevice9 * device)
{
	if (encodePipeline == nullptr) {
		encodePipeline = std::make_shared<Encode::D9EncodePipeline<EncType>>(device, this->pipe, this->socket);
	}
	return encodePipeline;
}

template <typename EncType>
std::shared_ptr<D9Hook<EncType>> D9Hook<EncType>::instance;

template <typename EncType>
std::shared_ptr<D9Hook<EncType>> D9Hook<EncType>::GetInstance()
{
	if (instance == nullptr) {
		instance = std::make_shared<D9Hook<EncType>>();
	}
	return instance;
};

template class D9Hook<Encode::AmdEncoder>;
template class D9Hook<Encode::NvidiaEncoder>;