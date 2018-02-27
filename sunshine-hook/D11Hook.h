#pragma once
#include "Hook.h"
#include "D11EncodePipeline.h"

typedef HRESULT(STDMETHODCALLTYPE * CREATE_DEVICE_AND_SWAP_TYPE)(
	IDXGIAdapter *pAdapter,
	D3D_DRIVER_TYPE DriverType,
	HMODULE Software,
	UINT Flags,
	const D3D_FEATURE_LEVEL *pFeatureLevels,
	UINT FeatureLevels,
	UINT SDKVersion,
	const DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
	IDXGISwapChain **ppSwapChain,
	ID3D11Device **ppDevice,
	D3D_FEATURE_LEVEL *pFeatureLevel,
	ID3D11DeviceContext **ppImmediateContext
	);

typedef HRESULT(STDMETHODCALLTYPE * PRESENT_SWAP_FUNC)(
	IDXGISwapChain * This,
	UINT SyncInterval,
	UINT Flags);

static HRESULT WINAPI HookPresent(
	IDXGISwapChain * This,
	UINT SyncInterval,
	UINT Flags);

class D11Hook : public Hook<Encode::D11EncodePipeline, ID3D11Device>
{
public:
	D11Hook();
	~D11Hook();
	std::shared_ptr<Encode::D11EncodePipeline> GetEncodePipeline(ID3D11Device * device) override;
	static std::shared_ptr<D11Hook> GetInstance();
	PRESENT_SWAP_FUNC GetPresent();
	bool Install();
	bool Uninstall();
private:
	static std::shared_ptr<D11Hook> instance;
	std::shared_ptr<Encode::D11EncodePipeline> encodePipeline;
	PRESENT_SWAP_FUNC presentSwap;
};

