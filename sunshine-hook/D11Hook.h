#pragma once
#include "GraphicHook.h"
#include "D11EncodePipeline.h"
#include "AmdEncoder.h"
#include "NvidiaEncoder.h"
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

template <typename EncType>
class D11Hook : public GraphicHook<Encode::D11EncodePipeline<EncType>, ID3D11Device>
{
public:
	D11Hook() {};
	std::shared_ptr<Encode::D11EncodePipeline<EncType>> GetEncodePipeline(ID3D11Device * device) override;
	
	static std::shared_ptr<D11Hook> GetInstance();
	PRESENT_SWAP_FUNC GetPresent();
	bool Install()
	{
		auto hMod = GetModuleHandle(TEXT("d3d11.dll"));
		if (hMod == nullptr) {
			hMod = LoadLibrary(TEXT("d3d11.dll"));
			if (hMod == nullptr) {
				LOG(ERROR) << "Load d3d11.dll module failed.";
				return false;
			}
		}

		auto pd3d = (CREATE_DEVICE_AND_SWAP_TYPE)GetProcAddress(hMod, "D3D11CreateDeviceAndSwapChain");
		if (pd3d == nullptr) {
			LOG(ERROR) << "GetProcAddress for D3D11CreateDeviceAndSwapChain failed";
			return false;
		}

		//	Create temporary window
		WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, TempWndProc,0,0,GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, TEXT("1"), nullptr };

		//	Register window
		RegisterClassEx(&wc);

		//	CreateWindow
		auto hWnd = CreateWindow(TEXT("1"), nullptr, WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, GetDesktopWindow(), nullptr, wc.hInstance, nullptr);
		if (hWnd == nullptr) {
			LOG(ERROR) << "CreateWindow failed. Error: " << GetLastError();;
			return false;
		}

		//	Set Swap chain description, just for initialization, does not really affect anything.
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 1;
		sd.BufferDesc.Width = 640;
		sd.BufferDesc.Height = 480;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		IDXGISwapChain * pSwapChain;
		ID3D11Device * device;
		ID3D11DeviceContext * context;
		D3D_FEATURE_LEVEL levels = D3D_FEATURE_LEVEL_11_0;
		D3D_FEATURE_LEVEL  FeatureLevelsSupported;

		// DIRECT3DCREATE11
		HRESULT d3d = pd3d(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &levels, 1, D3D11_SDK_VERSION, &sd, &pSwapChain, &device, &FeatureLevelsSupported, &context);
		if (d3d != S_OK) {
			LOG(ERROR) << "D3D11CreateDeviceAndSwapChain failed";
			return false;
		}

		//	Cleanup
		DestroyWindow(hWnd);

		//	Getting pointer to EndScene.
		uintptr_t * pInterface = (uintptr_t*) * (uintptr_t *)pSwapChain;

		//	Present: 8 (Source: Google)
		presentSwap = (PRESENT_SWAP_FUNC)pInterface[8];
		//	Install hook
		return this->InstallHook("SwapChainPresent", presentSwap, HookPresent);
	};
	bool Uninstall();
private:
	static std::shared_ptr<D11Hook> instance;
	std::shared_ptr<Encode::D11EncodePipeline<EncType>> encodePipeline;
	PRESENT_SWAP_FUNC presentSwap;
};

static HRESULT WINAPI HookPresent(
	IDXGISwapChain * This,
	UINT SyncInterval,
	UINT Flags)
{
	//	Perform backbuffer capture and encoding
#ifdef NVIDIA_ENC
	auto hook = D11Hook<Encode::NvidiaEncoder>::GetInstance();
#endif
#ifndef NVIDIA_ENC
	auto hook = D11Hook<Encode::AmdEncoder>::GetInstance();
#endif

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