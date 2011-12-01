/*
Copyright (c) 2011, Dilyan Rusev
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list 
of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, this list 
of conditions and the following disclaimer in the documentation and/or other materials 
provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE.
*/

#include "stdafx.h"
#include "Shlwapi.h"
#include "Resource.hxx"
#include "HotkeyRegistration.hxx"
#include "ComInit.hxx"
#include "AudioEndpointHandle.hxx"
#include "common.hxx"
#include "winbase.h"

#define WM_FOO_NOTIFY (WM_USER + 1)

// HIWORD(lparam) when messages is WM_FOO_NOTIFY
// {C7C2DDFE-76D5-400B-A14F-2284D60B2B8C}
static const GUID GUID_FOOMM_NOTIFYICON = 
{ 0xc7c2ddfe, 0x76d5, 0x400b, { 0xa1, 0x4f, 0x22, 0x84, 0xd6, 0xb, 0x2b, 0x8c } };

struct MainWindowData {	
	MainWindowData() 
			: pAudio(NULL)
			, hInst(NULL) {
	}

	AudioEndpointHandle *pAudio;
	NOTIFYICONDATA notify;	
	HINSTANCE hInst;
};

class AppLockMutex {
	HANDLE _hMutex;

public:	
	AppLockMutex() 
			: _hMutex(NULL) {
		_hMutex = CreateMutexEx(NULL, L"FooMMKeysAppLock", CREATE_MUTEX_INITIAL_OWNER, WRITE_OWNER);
		if (NULL == _hMutex) {
			throw std::runtime_error("Foo MM Keys already running");
		}
	}

	~AppLockMutex() {
		if (NULL != _hMutex) {
			ReleaseMutex(_hMutex);
		}
	}
};

///////////////////////////////////////////////////////////////////////////////
// Functions
///////////////////////////////////////////////////////////////////////////////
void show_last_error(const std::wstring& title);
int run_message_loop();
void handle_hotkey(Hotkeys hotkey);
HWND create_main_window(HINSTANCE hInst, MainWindowData *pAudio);
LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

///////////////////////////////////////////////////////////////////////////////
// WinMain
///////////////////////////////////////////////////////////////////////////////
int APIENTRY wWinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPWSTR lpCmdLine, int nCmdShow)
{
	try {		
		AppLockMutex appLock;
		ComInit comInit;
		AudioEndpointHandle audioHandle;			
		MainWindowData data;
		
		data.pAudio = &audioHandle;
		
		HWND hwnd = create_main_window(hInst, &data);

		HotkeyRegistration hkToggle(hwnd, Hotkey_ToggleVolume, MOD_ALT | MOD_CONTROL, VK_F10);
		HotkeyRegistration hkDown(hwnd, Hotkey_VolumeDown, MOD_ALT | MOD_CONTROL, VK_F11);
		HotkeyRegistration hkUp(hwnd, Hotkey_VolumeUp, MOD_ALT | MOD_CONTROL, VK_F12);
		HotkeyRegistration hkQuit(hwnd, Hotkey_Quit, MOD_ALT | MOD_CONTROL, VK_BACK);
		
		return run_message_loop();
	}
	catch (const std::exception &ex) {
		MessageBoxA(NULL, ex.what(), "Error!", MB_ICONERROR | MB_OK);
		return 1;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Function handle_hotkey(Hotkeys, AudioEndpointHandle*)
///////////////////////////////////////////////////////////////////////////////
void handle_hotkey(Hotkeys hotkey, AudioEndpointHandle *pAudio) {
	if (NULL == pAudio) {
		return;
	}

	switch (hotkey)	{
	case Hotkey_Quit:
		PostQuitMessage(0);
		break;

	case Hotkey_ToggleVolume:
		pAudio->toggle_mute();
		break;

	case Hotkey_VolumeDown:
		pAudio->decrease_volume();
		break;

	case Hotkey_VolumeUp:
		pAudio->increase_volume();
		break;

	default:
		log_message("Not implemented");
		break;
	}	
}

///////////////////////////////////////////////////////////////////////////////
// Main Window WNDPROC
///////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	static MainWindowData* pData = NULL;

	switch (msg) {
	case WM_CREATE:
	{
		LPCREATESTRUCT cs = (LPCREATESTRUCT)lparam;
		pData = (MainWindowData*)cs->lpCreateParams;
		break;
	}

	case WM_FOO_NOTIFY:
	{		
		switch (lparam)
		{
		case WM_LBUTTONDBLCLK:
			if (IsWindowVisible(hwnd)) {
				ShowWindow(hwnd, SW_HIDE);
			}
			else {
				ShowWindow(hwnd, SW_SHOW);
			}
			break;
		}
		break;
	}		

	case WM_HOTKEY:
		if (NULL != pData) {
			handle_hotkey((Hotkeys)wparam, pData->pAudio);
		}
		break;

	case WM_CLOSE:
		if (!Shell_NotifyIcon(NIM_DELETE, &pData->notify)) {
			show_last_error(L"Shell_NotifyIcon NIM_DELETE");
		}
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:		
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Function create_main_window(HINSTANCE, MainWindowData*)
///////////////////////////////////////////////////////////////////////////////

HWND create_main_window(HINSTANCE hInst, MainWindowData *pData) {
	if (NULL == pData) {
		throw std::invalid_argument("pData is NULL");
	}
	pData->hInst = hInst;

	WNDCLASSEX wc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.cbSize = sizeof(wc);
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_FOOMMKEYS));
	wc.hIconSm = LoadIcon(NULL, MAKEINTRESOURCE(IDI_SMALL));
	wc.hInstance = hInst;
	wc.lpfnWndProc = wnd_proc;
	wc.lpszClassName = L"FooMMKeys";
	wc.lpszMenuName = NULL;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;

	if (!RegisterClassEx(&wc)) {
		throw std::runtime_error("RegisterClassEx failed");
	}

	DWORD style = WS_MINIMIZEBOX | WS_CAPTION | WS_SYSMENU;
	RECT rt = {0, 0, 300, 300};
	AdjustWindowRect(&rt, style, FALSE);
	int width = rt.right - rt.left;
	int height = rt.bottom - rt.top;
	int screnWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	int posX = (screnWidth - width) / 2;
	int posY = (screenHeight - height) / 2;

	HWND hwnd = CreateWindow(L"FooMMKeys", L"Foo Multimedia Keys",
		style, posX, posY, width, height,
		HWND_DESKTOP, NULL, hInst, pData);
	if (NULL == hwnd) {
		throw std::runtime_error("CreateWindow failed");
	}
	
	DLLGETVERSIONPROC DllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(GetModuleHandle(L"Shell32.dll"), "DllGetVersion");
	if (NULL == DllGetVersion) {
		throw std::runtime_error("Can't get DllGetVersion from Shell32.dll");
	}
	DLLVERSIONINFO verInfo;
	verInfo.cbSize = sizeof(verInfo);
	DllGetVersion(&verInfo);
	
	NOTIFYICONDATA niData; 
	ZeroMemory(&niData,sizeof(NOTIFYICONDATA));

	if(verInfo.dwMajorVersion >= 6) {
		niData.cbSize = sizeof(NOTIFYICONDATA);
	}
	else if(verInfo.dwMajorVersion >= 5) {
		niData.cbSize = NOTIFYICONDATA_V2_SIZE;
	}
	else {
		niData.cbSize = NOTIFYICONDATA_V1_SIZE;
	}

	niData.uID = 1;
	niData.uFlags = NIF_ICON|NIF_MESSAGE|NIF_TIP;
	niData.hIcon =
		(HICON)LoadImage( hInst,
			MAKEINTRESOURCE(IDI_FOOMMKEYS),
			IMAGE_ICON,
			GetSystemMetrics(SM_CXSMICON),
			GetSystemMetrics(SM_CYSMICON),
			LR_DEFAULTCOLOR);
	niData.hWnd = hwnd;
	niData.uCallbackMessage = WM_FOO_NOTIFY;


	pData->notify = niData;
	
	if (!Shell_NotifyIcon(NIM_ADD, &pData->notify)) {
		
		show_last_error(L"Shell_NotifyIcon: NIM_ADD");
	}

	ShowWindow(hwnd, SW_HIDE);
	UpdateWindow(hwnd);

	return hwnd;
}

///////////////////////////////////////////////////////////////////////////////
// Function run_message_loop()
///////////////////////////////////////////////////////////////////////////////

int run_message_loop() {
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}


void show_last_error(const std::wstring& title) {
	DWORD errorCode = GetLastError();
	LPWSTR pBuffer = NULL;
	if (FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL, errorCode, 0,	(LPWSTR)&pBuffer, 1000, NULL) != 0)
	{
		MessageBox(NULL, pBuffer, title.c_str(), MB_ICONERROR | MB_OK);
		LocalFree(pBuffer);
	}
	else 
	{
		MessageBox(NULL, L"General error", title.c_str(), MB_ICONERROR | MB_OK);
	}
}