#include "stdafx.h"
#include "ControlHook.h"

typedef BOOL(STDMETHODCALLTYPE* GETCURSORPOSTYPE)(LPPOINT lpPoint);
typedef UINT(STDMETHODCALLTYPE* GETRAWINPUTDATATYPE)(
	HRAWINPUT hRawInput,
	UINT      uiCommand,
	LPVOID    pData,
	PUINT     pcbSize,
	UINT      cbSizeHeader
);
typedef SHORT(STDMETHODCALLTYPE* GETKEYSTATETYPE)(int vKey);
typedef UINT(STDMETHODCALLTYPE* GETRAWINPUTBUFFERTYPE)(
	PRAWINPUT pData,
	PUINT     pcbSize,
	UINT      cbSizeHeader
);
static BOOL WINAPI HookGetCursorPos(LPPOINT lpPoint)
{
	return true;
}

static UINT WINAPI HookGetRawInputData(
	HRAWINPUT hRawInput,
	UINT      uiCommand,
	LPVOID    pData,
	PUINT     pcbSize,
	UINT      cbSizeHeader
)
{
	return 0;
}

static SHORT HookGetKeyState(int nVirtKey)
{
	return 0;
}

static UINT HookGetRawInputBuffer(
	PRAWINPUT pData,
	PUINT     pcbSize,
	UINT      cbSizeHeader
)
{
	LOG(INFO) << "GETTING RAW INPUT BUFFER";

	if (pData == nullptr) {
		// Return the minimum required buffer size in pcbSize
		*pcbSize = sizeof(RAWINPUT);
		return 0;
	}

	RAWINPUT rawInput;
	rawInput.header.dwType = RIM_TYPEMOUSE;
	rawInput.header.wParam = RIM_INPUT;
	rawInput.header.hDevice = 0;
	rawInput.header.dwSize = sizeof(rawInput);

	// MOUSE POSITION
	rawInput.data.mouse.lLastX = rand() % 1000;
	rawInput.data.mouse.lLastY = rand() % 1000;

	// MOUSE BUTTONS
	rawInput.data.mouse.ulButtons = 0;
	rawInput.data.mouse.ulRawButtons = 0;
	rawInput.data.mouse.usButtonFlags = 0;

	// MOUSE EXTRA
	rawInput.data.mouse.ulExtraInformation = 0;
	rawInput.data.mouse.usButtonData = 0;

	// MOUSE TYPE OF MOVEMENT
	rawInput.data.mouse.usFlags = MOUSE_MOVE_ABSOLUTE;

	memcpy(pData, &rawInput, sizeof(rawInput));
	return sizeof(rawInput);
}

bool ControlHook::Install()
{
	HMODULE hMod = GetModuleHandle(TEXT("user32.dll"));
	if (hMod == nullptr) {
		LOG(ERROR) << "Load user32.dll module failed.";
		return false;
	}

	auto getCursorPos = reinterpret_cast<GETCURSORPOSTYPE>(GetProcAddress(hMod, "GetCursorPos"));
	if (getCursorPos == nullptr) {
		LOG(ERROR) << "GetProcAddress failed for GetCursorPos";
		return false;
	}

	this->InstallHook("GetCursorPos", getCursorPos, HookGetCursorPos);

	auto getRawInputData = reinterpret_cast<GETRAWINPUTDATATYPE>(GetProcAddress(hMod, "GetRawInputData"));
	if (getRawInputData == nullptr) {
		LOG(ERROR) << "GetProcAddress failed for GetRawInputData";
		return false;
	}

	this->InstallHook("GetRawInputData", getRawInputData, HookGetRawInputData);

	auto getKeyState = reinterpret_cast<GETKEYSTATETYPE>(GetProcAddress(hMod, "GetKeyState"));
	if (getKeyState == nullptr) {
		LOG(ERROR) << "GetProcAddress failed for GetKeyState";
		return false;
	}

	this->InstallHook("GetKeyState", getKeyState, HookGetKeyState);

	auto getAsyncKeyState = reinterpret_cast<GETKEYSTATETYPE>(GetProcAddress(hMod, "GetAsyncKeyState"));
	if (getAsyncKeyState == nullptr) {
		LOG(ERROR) << "GetProcAddress failed for GetAsyncKeyState";
		return false;
	}

	this->InstallHook("GetAsyncKeyState", getAsyncKeyState, HookGetKeyState);

	auto getRawInputBuffer = reinterpret_cast<GETRAWINPUTBUFFERTYPE>(GetProcAddress(hMod, "GetRawInputBuffer"));
	if (getRawInputBuffer == nullptr) {
		LOG(ERROR) << "GetProcAddress failed for GetRawInputBuffer";
		return false;
	}

	this->InstallHook("GetRawInputBuffer", getRawInputBuffer, HookGetRawInputBuffer);
	return true;
}