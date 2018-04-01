#pragma once
#include "GraphicHook.h"
#include <d3d9.h>
#include <memory>
#include "D9EncodePipeline.h"
#include "..\easyloggingpp\easylogging++.h"

typedef IDirect3D9 * (STDMETHODCALLTYPE * CREATE_DIRECT_TYPE)(UINT SDKVersion);
typedef HRESULT(WINAPI* END_SCENE_FUNC)(LPDIRECT3DDEVICE9 device);
HRESULT WINAPI HookEndScene(LPDIRECT3DDEVICE9 device);

template <typename EncType>
class D9Hook :
	public GraphicHook<Encode::D9EncodePipeline<EncType>, IDirect3DDevice9>
{
public:
	D9Hook() {};
	bool Install()
	{
		auto hMod = GetModuleHandle(TEXT("d3d9.dll"));
		if (hMod == nullptr) {
			hMod = LoadLibrary(TEXT("d3d9.dll"));
			if (hMod == nullptr) {
				LOG(ERROR) << "Load d3d9.dll module failed.";
				return false;
			}
		}

		auto pd3d = (CREATE_DIRECT_TYPE)GetProcAddress(hMod, "Direct3DCreate9");
		if (pd3d == nullptr) {
			LOG(ERROR) << "GetProcAddress for Direct3DCreate9 failed";
			return false;
		}

		// DIRECT3DCREATE9
		auto d3d = pd3d(D3D_SDK_VERSION);
		if (d3d == nullptr) {
			LOG(ERROR) << "Direct3DCreate9 failed";
			return false;
		}

		//	Get adapter display  mode
		D3DDISPLAYMODE displayMode;
		auto adapterDisplayMode = d3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode);
		if (adapterDisplayMode != D3D_OK) {
			LOG(ERROR) << "GetAdapterDisplayMode failed";
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

		// Setting up PRESENT call params, might be useless (test removal).
		D3DPRESENT_PARAMETERS params;
		ZeroMemory(&params, sizeof(params));
		params.Windowed = true;
		params.SwapEffect = D3DSWAPEFFECT_DISCARD;
		params.BackBufferFormat = displayMode.Format;
		params.hDeviceWindow = hWnd;

		//	Create temporary device.
		LPDIRECT3DDEVICE9 deviceInterface;
		ZeroMemory(&deviceInterface, sizeof(deviceInterface));
		auto hres = d3d->CreateDevice(
			D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL,
			hWnd,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING,
			&params,
			&deviceInterface);
		if (hres != D3D_OK) {
			LOG(ERROR) << "IDirect3D9::CreateDevice failed.";
			return false;
		}

		//	Cleanup
		d3d->Release();
		DestroyWindow(hWnd);

		//	Getting pointer to EndScene.
		uintptr_t * pInterface = (uintptr_t*) * (uintptr_t *)deviceInterface;

		//	EndScene: 42 (Source: Google)
		endScene = (END_SCENE_FUNC)pInterface[42];

		//	Install hook
		return this->InstallHook("Direct3DCreate9", endScene, HookEndScene);
	};
	bool Uninstall();
	END_SCENE_FUNC GetEndScene()
	{
		if (endScene == nullptr) {
			// Should probably throw an exception.
			LOG(ERROR) << "Illegal call: EndScene ptr is null";
			ExitProcess(1);
		}
		return endScene;
	};
	std::shared_ptr<Encode::D9EncodePipeline<EncType>> GetEncodePipeline(IDirect3DDevice9 * device) override;
	static std::shared_ptr<D9Hook<EncType>> GetInstance();
private:
	static std::shared_ptr<D9Hook<EncType>> instance;
	END_SCENE_FUNC endScene;
	std::shared_ptr<Encode::D9EncodePipeline<EncType>> encodePipeline;
};
