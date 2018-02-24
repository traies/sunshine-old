#pragma once
#include "Hook.h"
#include <d3d9.h>
#include <memory>

typedef IDirect3D9 * (STDMETHODCALLTYPE * CREATE_DIRECT_TYPE)(UINT SDKVersion);
typedef HRESULT(WINAPI* END_SCENE_FUNC)(LPDIRECT3DDEVICE9 device);
HRESULT WINAPI HookEndScene(LPDIRECT3DDEVICE9 device);

class D9Hook :
	public Hook
{
public:
	D9Hook() {};
	~D9Hook() {};
	bool Install();
	bool Uninstall();
	END_SCENE_FUNC GetEndScene();
	static std::shared_ptr<D9Hook> GetInstance();
private:
	static std::shared_ptr<D9Hook> instance;
	END_SCENE_FUNC endScene;
};

