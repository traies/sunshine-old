#include "stdafx.h"
#include "D9Hook.h"
#include <d3d9.h>
#include "AmdEncoder.h"
#include "NvidiaEncoder.h"
#include "Bootstrap.h"

#pragma comment (lib, "d3d9.lib")

static HRESULT WINAPI HookEndScene(LPDIRECT3DDEVICE9 device)
{
	auto hook = D9Hook::GetInstance();

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

bool D9Hook::Uninstall()
{
	return false;
}

std::shared_ptr<Encode::D9EncodePipeline> D9Hook::GetEncodePipeline(IDirect3DDevice9 * device)
{
	if (encodePipeline == nullptr) {
		_encoder->Init(device);
		encodePipeline = std::make_shared<Encode::D9EncodePipeline>(std::move(_encoder), this->pipe, this->socket);
	}
	return encodePipeline;
}

D9Hook*  D9Hook::GetInstance()
{
	if (instance == nullptr) {
		LOG(ERROR) << "Tried to get d9 hook without setting it up first.";
	}
	return instance;
};

D9Hook* D9Hook::CreateInstance(std::unique_ptr<Encode::Encoder> encoder)
{
	instance = new D9Hook(std::move(encoder));
	return instance;
}

D9Hook* D9Hook::instance;
