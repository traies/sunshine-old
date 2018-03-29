// DXD9TestApplication.cpp : Defines the entry point for the application.

#include "stdafx.h"
#include "DXD9TestApplication.h"
#include <d3d9.h>
#include <D3dx9core.h>
#include <string>

#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>  

#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")
#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
LPDIRECT3D9 d3d;								// the pointer to our Direct3D interface
LPDIRECT3DDEVICE9 d3ddev;						// the pointer to our device class
LPD3DXFONT pFont;
int height;
int width;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int, HWND*);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void initD3D(HWND hWnd);						// sets up and initializes Direct3D
void render_frame(void);						// renders a single frame
void cleanD3D(void);							// closes Direct3D and releases memory
void drawText(int x, int y, DWORD color, const char * str);
inline void initFont();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_DXD9TESTAPPLICATION, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	HWND hWnd;
	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow, &hWnd))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DXD9TESTAPPLICATION));

	initD3D(hWnd);
	initFont();

	RECT r;
	GetClientRect(hWnd, &r);
	width = r.left - r.right;
	height = r.bottom - r.top;
	MSG msg;

	// Main message loop:
	while (TRUE)
	{
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		if (msg.message == WM_QUIT) {
			break;
		}
		render_frame();
	}
	cleanD3D();
	return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDC_DXD9TESTAPPLICATION));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_DXD9TESTAPPLICATION);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow, HWND * hWnd)
{
	hInst = hInstance; // Store instance handle in our global variable

	*hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(*hWnd, nCmdShow);
	UpdateWindow(*hWnd);


	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_KEYDOWN:
	{
		break;
	}
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			ExitProcess(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void initD3D(HWND hWnd)
{
	d3d = Direct3DCreate9(D3D_SDK_VERSION);		// create the Direct3D interface
	D3DPRESENT_PARAMETERS d3dpp;				// create a struct to hold various device information

	ZeroMemory(&d3dpp, sizeof(d3dpp));			// clear out the struct for use
	d3dpp.Windowed = TRUE;						// program windowed, not fullscreen
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;	// discard old frames
	d3dpp.hDeviceWindow = hWnd;					// set the window to be used by Direct3D
	//d3dpp.BackBufferFormat = d3dfmt_;	// create a device class using this information and information from the d3dpp struct
	auto hres = d3d->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp,
		&d3ddev
	);
}


void render_frame(void)
{
	static int f = 0;
	f++;
	// clear the window to a deep blue
	d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 40, 100), 1.0f, 0);

	d3ddev->BeginScene();						// begins the 3D scene

												// do 3D rendering on the back buffer here


	drawText(0, 0, D3DCOLOR_XRGB(255, 255, 255), std::to_string(f).c_str());
	drawText(100, 0, D3DCOLOR_XRGB(255, 255, 255), std::to_string(width).c_str());
	drawText(200, 0, D3DCOLOR_XRGB(255, 255, 255), std::to_string(height).c_str());
	d3ddev->EndScene();							// end the 3D scene

	d3ddev->Present(NULL, NULL, NULL, NULL);	// displays the created frame

}

// this is the function that cleans up Direct3D and COM
void cleanD3D(void)
{
	d3ddev->Release();		// close and release the 3D device
	d3d->Release();			// close and release Direct3D
}


inline void initFont()
{
	HRESULT r = 0;

	HFONT hFont = (HFONT)GetStockObject(SYSTEM_FONT);
	pFont = 0;

	r = D3DXCreateFont(d3ddev, 24, 0, 0, 1, false, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"hello", &pFont);
	if (FAILED(r)) {
		return;
	}

}

void drawText(int x, int y, DWORD color, const char * str)
{
	if (!pFont) {
		return;
	}
	int textWidth = 24 * strlen(str);
	int textHeight = 24;
	RECT rect = { x, y, x + textWidth, y + textHeight };
	DWORD format = DT_EXPANDTABS;
	pFont->DrawTextA(NULL, str, -1, &rect, format, color);
}