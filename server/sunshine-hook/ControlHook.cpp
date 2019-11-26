#include "stdafx.h"
#include "ControlHook.h"
#include "MouseShadowController.h"
#include "KeyboardShadowController.h"

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

typedef UINT(STDMETHODCALLTYPE* GETRAWINPUTDEVICELIST)(
	PRAWINPUTDEVICELIST pRawInputDeviceList,
	PUINT               puiNumDevices,
	UINT                cbSize
	);

static BOOL HookRegisterRawInputDevices(
	PCRAWINPUTDEVICE pRawInputDevices,
	UINT uiNumDevices,
	UINT cbSize
)
{
	LOG(INFO) << "REGISTER RAW INPUT DEVICES";
	return true;
};


static BOOL HookGetInputState()
{
	LOG(INFO) << "GET INPUT STATE";
	return true;
}

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

static GETRAWINPUTDEVICELIST getRawInputDeviceList = nullptr;
static UINT HookGetRawInputDeviceList(
	PRAWINPUTDEVICELIST pRawInputDeviceList,
	PUINT               puiNumDevices,
	UINT                cbSize
)
{
	RAWINPUTDEVICELIST devices[256];

	auto keyController = KeyboardShadowController::GetInstance();
	int keyboardCommands = min(keyController->lastCommand, 128);
	auto mouseController = MouseShadowController::GetInstance();
	int mouseCommands = min(mouseController->lastCommand, 128);

	if (!pRawInputDeviceList) {
		*puiNumDevices = keyboardCommands + mouseCommands;
	}

	int keyRightIndex = min(keyboardCommands, *puiNumDevices);

	for (int i = 0; i < keyRightIndex; i++) {
		devices[i].dwType = RIM_TYPEKEYBOARD;
		devices[i].hDevice = (HANDLE)(i);
	}

	for (int i = 0; i < min(mouseCommands, *puiNumDevices); i++) {
		devices[keyRightIndex + i].dwType = RIM_TYPEMOUSE;
		devices[keyRightIndex + i].hDevice = (HANDLE)(128 + i);
	}


	if (pRawInputDeviceList != nullptr) {
		LOG(INFO) << "GET RAW INPUT DEVICE LIST NOT NULL";
		memcpy(pRawInputDeviceList, devices, sizeof(RAWINPUTDEVICELIST) * (*puiNumDevices));
		return *puiNumDevices;
	}
	LOG(INFO) << "GET RAW INPUT DEVICE LIST WITH NULL";

	/*if (getRawInputDeviceList) {
		auto res = getRawInputDeviceList(
			pRawInputDeviceList,
			puiNumDevices,
			cbSize
		);

		return res;
	}*/
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
	return true;
}

static void FillMouseRawInputHeader(RAWINPUTHEADER& header, size_t rawInputSize, LPARAM handle)
{
	header.dwType = RIM_TYPEMOUSE;
	header.wParam = RIM_INPUT;
	header.hDevice = (HANDLE)handle;
	header.dwSize = rawInputSize;
}

static void FillKeyRawInputHeader(RAWINPUTHEADER& header, size_t rawInputSize, LPARAM handle)
{
	header.dwType = RIM_TYPEKEYBOARD;
	header.wParam = RIM_INPUT;
	header.hDevice = (HANDLE)handle;
	header.dwSize = rawInputSize;
}


inline static void FillMouseRawInput(RAWINPUT& rawInput, RAWMOUSE rawMouse, LPARAM handle)
{
	FillMouseRawInputHeader(rawInput.header, sizeof(RAWINPUT), handle);
	rawInput.data.mouse = rawMouse;
}

inline static void FillKeyRawInput(RAWINPUT& rawInput, InputCommand command, LPARAM handle)
{
	FillKeyRawInputHeader(rawInput.header, sizeof(RAWINPUT), handle);

	rawInput.data.keyboard.VKey = command.val1;
	rawInput.data.keyboard.MakeCode = command.val2;
	rawInput.data.keyboard.Reserved = 0;
	if (command.event1 == (int16_t)ButtonEventType::BUTTON_EVENT_DOWN) {
		rawInput.data.keyboard.Flags = RI_KEY_MAKE;
		rawInput.data.keyboard.Message = WM_KEYDOWN;
	}
	else {
		//ButtonEventType::BUTTON_EVENT_UP
		rawInput.data.keyboard.Flags = RI_KEY_BREAK;
		rawInput.data.keyboard.Message = WM_KEYUP;
	}
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
		const char* fakeName;
		if ((int)hDevice >= 128) {
			fakeName = "FakeMouse";
		}
		else {
			fakeName = "FakeKeyboard";
		}
		*pcbSize = strlen(fakeName);
		buf = (void *)fakeName;
	}
	else if (uiCommand == RIDI_DEVICEINFO) {
		RID_DEVICE_INFO deviceInfo;
		ZeroMemory(&deviceInfo, sizeof(deviceInfo));
		if ((int)hDevice >= 128) {
			deviceInfo.cbSize = sizeof(deviceInfo);
			deviceInfo.dwType = RIM_TYPEMOUSE;
			deviceInfo.mouse.dwId = 5;
			deviceInfo.mouse.dwNumberOfButtons = 5;
			deviceInfo.mouse.dwSampleRate = 100;
			deviceInfo.mouse.fHasHorizontalWheel = false;
		}
		else {
			deviceInfo.cbSize = sizeof(deviceInfo);
			deviceInfo.dwType = RIM_TYPEKEYBOARD;
			deviceInfo.keyboard.dwType = 81;
			deviceInfo.keyboard.dwSubType = 0;
			deviceInfo.keyboard.dwKeyboardMode = 1;
			deviceInfo.keyboard.dwNumberOfFunctionKeys = 12;
			deviceInfo.keyboard.dwNumberOfIndicators = 3;
			deviceInfo.keyboard.dwNumberOfKeysTotal = 173;
		}
		*pcbSize = sizeof(deviceInfo);
		buf = &deviceInfo;
	}
	else {
		  if (uiCommand == RIDI_PREPARSEDDATA) 
		LOG(INFO) << "Calling get Raw Input Device Info RIDI_PREPARSEDDATA";
		buf = (void*) "";
		*pcbSize = 0;
	}

	if (pData != nullptr) {
		memcpy(pData, buf, *pcbSize);
		return *pcbSize;
	}

	return 0;
	//if (getRawInputDeviceInfoW) {
	//	auto res = getRawInputDeviceInfoW(
	//		hDevice,
	//		uiCommand,
	//		pData,
	//		pcbSize
	//	);
	//	return res;
	//}
	//else {
	//	return 0;
	//}

}

static GETRAWINPUTDATATYPE getRawInputData = nullptr;

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
		LPARAM handle = (LPARAM) hRawInput;
		if (pData != nullptr) {
			if (handle >= 128) {
				auto controller = MouseShadowController::GetInstance();
				RAWMOUSE rawMouse = controller->GetRawMouse(hRawInput);
				FillMouseRawInput(rawInput, rawMouse, handle);
			}
			else {
				auto controller = KeyboardShadowController::GetInstance();
				InputCommand command = controller->GetInputCommand(hRawInput);
				FillKeyRawInput(rawInput, command, handle);
			}
		}
		buff = &rawInput;
		*pcbSize = sizeof(RAWINPUT);
	}
	else {
		// uiCommand == RID_HEADER
		RAWINPUTHEADER rawInputHeader;
		ZeroMemory(&rawInputHeader, sizeof(rawInputHeader));
		LPARAM handle = (LPARAM)hRawInput;
		if (handle >= 128) {
			FillMouseRawInputHeader(rawInputHeader, sizeof(RAWINPUT), handle);
		}
		else {
			FillKeyRawInputHeader(rawInputHeader, sizeof(RAWINPUT), handle);
		}
		buff = &rawInputHeader;
		*pcbSize = sizeof(RAWINPUTHEADER);
	}
	if (pData != nullptr) {
		memcpy(pData, buff, *pcbSize);
		return *pcbSize;
	}
	return 0;
	
	
	/*if (getRawInputData) {
		auto res = getRawInputData(
			hRawInput,
			uiCommand,
			pData,
			pcbSize,
			cbSizeHeader
		);
		return res;
	}
	else {
		return 0;
	}*/

}

static SHORT HookGetAsyncKeyState(int nVirtKey)
{
	auto instance = KeyboardShadowController::GetInstance();
	auto ev = instance->keys[nVirtKey];
	if (ev == ButtonEventType::BUTTON_EVENT_DOWN) {
		return 0x8000;
	}
	else {
		auto instance = MouseShadowController::GetInstance();
		auto ev = instance->keys[nVirtKey];
		if (ev == ButtonEventType::BUTTON_EVENT_DOWN) {
			return 0x8000;
		}
		else {
			return 0x0000;
		}
	}
}

static SHORT HookGetKeyState(int nVirtKey)
{
	return HookGetAsyncKeyState(nVirtKey);
}

static UINT HookGetRawInputBuffer(
	PRAWINPUT pData,
	PUINT     pcbSize,
	UINT      cbSizeHeader
)
{
	//RAWINPUT rawInput[11];
	//ZeroMemory(&rawInput, sizeof(rawInput));
	//FillMouseRawInput(rawInput[0]);

	//auto controller = KeyboardShadowController::GetInstance();
	//InputCommand command[10];
	//int keyboardCommandsRead = controller->ReadInput(command,10);
	//for (int i = 0; i < keyboardCommandsRead; i++) {
	//	FillKeyRawInput(rawInput[1+i], command[i], i + 2);
	//}
	//
	//*pcbSize = sizeof(rawInput) * (1 + keyboardCommandsRead);
	//if (pData != nullptr) {
	//	memcpy(pData, &rawInput, *pcbSize);
	//}
	//return *pcbSize;
	LOG(INFO) << "CALLING GET RAW INPUT BUFFER ===> FIX";
	return 0;
}

bool ControlHook::Install()
{
	//RAWINPUTDEVICE devices[100];
	//UINT puiNumDevices = 100;
	//UINT found = GetRegisteredRawInputDevices(devices, &found, sizeof(RAWINPUTDEVICE));
	//if (found >= 0) {
	//	for (int i = 0; i < found; i++) {
	//		devices[i].dwFlags = RIDEV_REMOVE;
	//	}
	//	RegisterRawInputDevices(devices, found, sizeof(RAWINPUTDEVICE));
	//	LOG(INFO) << "UNREGISTERED " << found << " DEVICES";
	//}


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

	getRawInputData = reinterpret_cast<GETRAWINPUTDATATYPE>(GetProcAddress(hMod, "GetRawInputData"));
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
	
	if (getMouseMovePointsEx == nullptr) {
		LOG(ERROR) << "GetProcAddress failed for GetMouseMovePointsEx";
		return false;
	}

	this->InstallHook("GetMouseMovePointsEx", getMouseMovePointsEx, HookMouseMovePointsEx);

	auto getRegisteredRawInputDevices = (GetProcAddress(hMod, "GetRegisteredRawInputDevices"));
	if (getRegisteredRawInputDevices == nullptr) {
		LOG(ERROR) << "GetProcAddress failed for getRegisteredRawInputDevices";
		return false;
	}
	this->InstallHook("getRegisteredRawInputDevices", getRegisteredRawInputDevices, HookGetRegisteredRawInputDevices);	
	
	getRawInputDeviceList = reinterpret_cast<GETRAWINPUTDEVICELIST>(GetProcAddress(hMod, "GetRawInputDeviceList"));
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

	this->InstallHook("GetAsyncKeyState", getAsyncKeyState, HookGetAsyncKeyState);

	//auto registerRawInputDevices = GetProcAddress(hMod, "RegisterRawInputDevices");
	//if (registerRawInputDevices == nullptr) {
	//	LOG(ERROR) << "GetProcAddress failed for RegisterRawInputDevices";
	//	return false;
	//}

	//this->InstallHook("RegisterRawInputDevices", registerRawInputDevices, HookRegisterRawInputDevices);
	
	auto getInputState = GetProcAddress(hMod, "GetInputState");
	if (getInputState == nullptr) {
		LOG(ERROR) << "GetProcAddress failed for GetInputState";
		return false;
	}

	this->InstallHook("GetInputState", getInputState, HookGetAsyncKeyState);


	return true; 
}