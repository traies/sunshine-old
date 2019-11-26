#include "stdafx.h"
#include "D11Hook.h"
#include "AmdEncoder.h"
#include "NvidiaEncoder.h"
#include "Bootstrap.h"

static HRESULT WINAPI HookPresent(
	IDXGISwapChain * This,
	UINT SyncInterval,
	UINT Flags)
{
	//	Perform backbuffer capture and encoding
	auto hook = D11Hook::GetInstance();

	ID3D11Device * device;
	This->GetDevice(__uuidof(device), (void **)&device);
	auto pipeline = hook->GetEncodePipeline(device);

	//	Get backbuffer.
	ID3D11Texture2D * backbuffer;
	This->GetBuffer(0, __uuidof(backbuffer), (void **)&backbuffer);
	if (!pipeline->Call(backbuffer)) {
		LOG(ERROR) << "EncodePipeline failed.";
		//	ExitProcess(1);
	}
	return hook->GetPresent()(This, SyncInterval, Flags);
}

PRESENT_SWAP_FUNC D11Hook::GetPresent()
{
	if (presentSwap == nullptr) {
		// Should probably throw an exception.
		LOG(ERROR) << "Illegal call: Present ptr is null";
		ExitProcess(1);
	}
	return presentSwap;
}

std::shared_ptr<Encode::D11EncodePipeline> D11Hook::GetEncodePipeline(ID3D11Device * device)
{
	if (encodePipeline == nullptr) {
		_encoder->Init(device);
		encodePipeline = std::make_shared<Encode::D11EncodePipeline>(std::move(_encoder), this->pipe, this->socket);
	}
	return encodePipeline;
}

bool D11Hook::Uninstall()
{
	return false;
}

D11Hook * D11Hook::GetInstance()
{
	if (instance == nullptr) {
		LOG(ERROR) << "Tried to get d11 hook without setting it up first.";
	}
	return instance;
};

D11Hook* D11Hook::CreateInstance(std::unique_ptr<Encode::Encoder> encoder)
{
	instance = new D11Hook(std::move(encoder));
	return instance;
}

D11Hook* D11Hook::instance;