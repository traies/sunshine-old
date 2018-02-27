#include "stdafx.h"
#include "D11Hook.h"

bool D11Hook::Install()
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
	return InstallHook("SwapChainPresent", presentSwap, HookPresent);
}

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
		ExitProcess(1);
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

D11Hook::D11Hook()
{
}

std::shared_ptr<D11Hook> D11Hook::instance;

std::shared_ptr<D11Hook> D11Hook::GetInstance()
{
	if (instance == nullptr) {
		instance = std::make_shared<D11Hook>();
	}
	return instance;
}

std::shared_ptr<Encode::D11EncodePipeline> D11Hook::GetEncodePipeline(ID3D11Device * device)
{
	if (encodePipeline == nullptr) {
		encodePipeline = std::make_shared<Encode::D11EncodePipeline>(device, pipe);
	}
	return encodePipeline;
}

D11Hook::~D11Hook()
{
}

bool D11Hook::Uninstall()
{
	return false;
}