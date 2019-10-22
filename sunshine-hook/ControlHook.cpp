#include "stdafx.h"
#include "ControlHook.h"
#include "MouseShadowController.h"

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

typedef UINT(STDMETHODCALLTYPE* GETRAWINPUTDEVICEINFO)(
	HANDLE hDevice,
	UINT uiCommand,
	LPVOID pData,
	PUINT pcbSize
);

static BOOL HookGetLastInput(
	PLASTINPUTINFO  plii
)
{
	LOG(INFO) << "GET LAST INPUT INFO";
	plii->cbSize = sizeof(LASTINPUTINFO);
	plii->dwTime = 100;
	return true;
}

static UINT HookGetRegisteredRawInputDevices(
	PRAWINPUTDEVICE pRawInputDevices,
	PUINT puiNumDevices,
	UINT cbSize
)
{
	LOG(INFO) << "GET REGISTERED RAW INPUT DEVICES";
	return 0;
}

static UINT HookGetRawInputDeviceList(
	PRAWINPUTDEVICELIST pRawInputDeviceList,
	PUINT               puiNumDevices,
	UINT                cbSize
)
{
	LOG(INFO) << "GET RAW INPUT DEVICE LIST";
	return 0;
}

static int HookMouseMovePointsEx(
	UINT cbSize, 
	LPMOUSEMOVEPOINT lppt, 
	LPMOUSEMOVEPOINT lpptBuf, 
	int nBufPoints, 
	DWORD resolution
)
{
	LOG(INFO) << "Mouse move points";
	return 0;
}

static BOOL WINAPI HookGetCursorPos(LPPOINT lpPoint)
{
	auto controller = MouseShadowController::GetInstance();
	lpPoint->x = controller->lastX;
	lpPoint->y = controller->lastY;
	//LOG(INFO) << "GET CURSOR POS";
	return true;
}

static void FillMouseRawInputHeader(RAWINPUTHEADER& header, size_t rawInputSize)
{
	header.dwType = RIM_TYPEMOUSE;
	header.wParam = RIM_INPUT;
	header.hDevice = (HANDLE)1;
	header.dwSize = rawInputSize;
}

static USHORT ButtonFlags()
{
	auto controller = MouseShadowController::GetInstance();
	USHORT buttonFlags = controller->lb ? RI_MOUSE_LEFT_BUTTON_DOWN : RI_MOUSE_LEFT_BUTTON_UP;
	buttonFlags |= controller->rb ? RI_MOUSE_RIGHT_BUTTON_DOWN : RI_MOUSE_RIGHT_BUTTON_UP;
	buttonFlags |= controller->mb ? RI_MOUSE_MIDDLE_BUTTON_DOWN : RI_MOUSE_MIDDLE_BUTTON_UP;
	buttonFlags |= controller->x1b ? RI_MOUSE_BUTTON_4_DOWN : RI_MOUSE_BUTTON_4_UP;
	buttonFlags |= controller->x2b ? RI_MOUSE_BUTTON_5_DOWN : RI_MOUSE_BUTTON_5_UP;
	buttonFlags |= RI_MOUSE_WHEEL;
	return buttonFlags;
}

static void FillMouseRawInput(RAWINPUT& rawInput)
{
	FillMouseRawInputHeader(rawInput.header, sizeof(RAWINPUT));

	auto controller = MouseShadowController::GetInstance();
	// MOUSE POSITION
	rawInput.data.mouse.lLastX = controller->lastX;
	rawInput.data.mouse.lLastY = controller->lastY;

	// MOUSE BUTTONS
	// Could not find information online, assume it is unused
	rawInput.data.mouse.ulRawButtons = 0;

	rawInput.data.mouse.usButtonFlags = ButtonFlags();
	rawInput.data.mouse.usButtonData = controller->scrollOffset;

	// MOUSE EXTRA
	rawInput.data.mouse.ulExtraInformation = 0;

	// MOUSE TYPE OF MOVEMENT
	rawInput.data.mouse.usFlags = MOUSE_MOVE_ABSOLUTE;
}

static GETRAWINPUTDEVICEINFO getRawInputDeviceInfoW = nullptr;
static UINT WINAPI HookGetRawInputDeviceInfoW(
	HANDLE hDevice,
	UINT uiCommand,
	LPVOID pData,
	PUINT pcbSize
)
{
	void * buf;
	if (uiCommand == RIDI_DEVICENAME) {
		LOG(INFO) << "Calling Get Raw Input Device Info RIDI_DEVICENAME";
		const char* fakeName = "FakeMouse";
		*pcbSize = strlen(fakeName);
		buf = (void *)fakeName;
	}
	else if (uiCommand == RIDI_DEVICEINFO) {
		LOG(INFO) << "Calling Get Raw Input Device Info RIDI_DEVICEINFO";
		RID_DEVICE_INFO deviceInfo;
		ZeroMemory(&deviceInfo, sizeof(deviceInfo));
		deviceInfo.cbSize = sizeof(deviceInfo);
		deviceInfo.dwType = RIM_TYPEMOUSE;
		deviceInfo.mouse.dwId = 5;
		deviceInfo.mouse.dwNumberOfButtons = 0;
		deviceInfo.mouse.dwSampleRate = 100;
		deviceInfo.mouse.fHasHorizontalWheel = TRUE;
		*pcbSize = sizeof(deviceInfo);
		buf = &deviceInfo;
	}
	else {
		//  if (uiCommand == RIDI_PREPARSEDDATA) 
		LOG(INFO) << "Calling get Raw Input Device Info RIDI_PREPARSEDDATA";
		buf = (void*) "";
		*pcbSize = 0;
	}

	if (pData != nullptr) {
		memcpy(pData, buf, *pcbSize);
		return *pcbSize;
	}
	return 0;
}

static UINT WINAPI HookGetRawInputData(
	HRAWINPUT hRawInput,
	UINT      uiCommand,
	LPVOID    pData,
	PUINT     pcbSize,
	UINT      cbSizeHeader
)
{
	LPVOID buff;
	if (uiCommand == RID_INPUT) {
		RAWINPUT rawInput;
		ZeroMemory(&rawInput, sizeof(rawInput));
		FillMouseRawInput(rawInput);
		buff = &rawInput;
		*pcbSize = sizeof(RAWINPUT);
	}
	else {
		// uiCommand == RID_HEADER
		RAWINPUTHEADER rawInputHeader;
		ZeroMemory(&rawInputHeader, sizeof(rawInputHeader));
		FillMouseRawInputHeader(rawInputHeader, sizeof(RAWINPUT));
		buff = &rawInputHeader;
		*pcbSize = sizeof(RAWINPUTHEADER);
	}
	if (pData != nullptr) {
		memcpy(pData, buff, *pcbSize);
		return *pcbSize;
	}
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
	ZeroMemory(&rawInput, sizeof(rawInput));
	FillMouseRawInput(rawInput);

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

	auto getRawInputBuffer = reinterpret_cast<GETRAWINPUTBUFFERTYPE>(GetProcAddress(hMod, "GetRawInputBuffer"));
	if (getRawInputBuffer == nullptr) {
		LOG(ERROR) << "GetProcAddress failed for GetRawInputBuffer";
		return false;
	}

	this->InstallHook("GetRawInputBuffer", getRawInputBuffer, HookGetRawInputBuffer);

	getRawInputDeviceInfoW = reinterpret_cast<GETRAWINPUTDEVICEINFO>(GetProcAddress(hMod, "GetRawInputDeviceInfoW"));
	if (getRawInputDeviceInfoW == nullptr) {
		LOG(ERROR) << "GetProcAddress failed for GetRawInputDeviceInfoW";
		return false;
	}

	this->InstallHook("GetRawInputDeviceInfoW", getRawInputDeviceInfoW, HookGetRawInputDeviceInfoW);

	auto getRawInputDeviceInfoA = reinterpret_cast<GETRAWINPUTDEVICEINFO>(GetProcAddress(hMod, "GetRawInputDeviceInfoA"));
	if (getRawInputDeviceInfoA == nullptr) {
		LOG(ERROR) << "GetProcAddress failed for GetRawInputDeviceInfoA";
		return false;
	}

	this->InstallHook("GetRawInputDeviceInfoA", getRawInputDeviceInfoA, HookGetRawInputDeviceInfoW);
	
	auto getMouseMovePointsEx = GetProcAddress(hMod, "GetMouseMovePointsEx");
	
	if (getRawInputDeviceInfoW == nullptr) {
		LOG(ERROR) << "GetProcAddress failed for GetMouseMovePointsEx";
		return false;
	}

	this->InstallHook("GetMouseMovePointsEx", getMouseMovePointsEx, HookMouseMovePointsEx);

	auto getRegisteredRawInputDevices = GetProcAddress(hMod, "GetRegisteredRawInputDevices");
	if (getRegisteredRawInputDevices == nullptr) {
		LOG(ERROR) << "GetProcAddress failed for getRegisteredRawInputDevices";
		return false;
	}
	this->InstallHook("getRegisteredRawInputDevices", getRegisteredRawInputDevices, HookGetRegisteredRawInputDevices);	
	
	auto getRawInputDeviceList = GetProcAddress(hMod, "GetRawInputDeviceList");
	if (getRawInputDeviceList == nullptr) {
		LOG(ERROR) << "GetProcAddress failed for getRawInputDeviceList";
		return false;
	}
	this->InstallHook("getRawInputDeviceList", getRawInputDeviceList, HookGetRawInputDeviceList);

	auto getLastInputInfo = GetProcAddress(hMod, "GetLastInputInfo");
	if (getLastInputInfo == nullptr) {
		LOG(ERROR) << "GetProcAddress failed for getLastInputInfo";
		return false;
	}
	this->InstallHook("getLastInputInfo", getLastInputInfo, HookGetLastInput);

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


	return true;
}