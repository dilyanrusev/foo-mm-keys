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
#include "Resource.hxx"
#include "HotkeyRegistration.hxx"
#include "ComInit.hxx"
#include "AudioEndpointHandle.hxx"
#include "common.hxx"

#define WM_FOO_NOTIFY (WM_USER + 1)

// HIWORD(lparam) when messages is WM_FOO_NOTIFY
// {7A5F0E4B-CE5E-46A3-94BA-663F1746CAE5}
static const GUID GUID_FOOMM_NOTIFYICON = 
{ 0x7a5f0e4b, 0xce5e, 0x46a3, { 0x94, 0xba, 0x66, 0x3f, 0x17, 0x46, 0xca, 0xe5 } };


struct MainWindowData {	
	MainWindowData() 
			: pAudio(NULL)
			, hInst(NULL) {
	}

	AudioEndpointHandle *pAudio;
	NOTIFYICONDATA notify;	
	HINSTANCE hInst;
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
				
		ZeroMemory(&pData->notify, sizeof(NOTIFYICONDATA));
		pData->notify.cbSize = sizeof(NOTIFYICONDATA);
		pData->notify.hWnd = hwnd;
		pData->notify.uFlags = NIF_ICON | NIF_TIP | NIF_GUID | NIF_MESSAGE;
		pData->notify.guidItem = GUID_FOOMM_NOTIFYICON;
		pData->notify.uCallbackMessage = WM_FOO_NOTIFY;		
		pData->notify.uVersion = NOTIFYICON_VERSION_4;
		StringCchCopy(pData->notify.szTip, ARRAYSIZE(pData->notify.szTip), L"Foo MM Keys: double click to toggle visibility");
		pData->notify.hIcon = LoadIcon(pData->hInst, MAKEINTRESOURCE(IDI_SMALL));	


		if (!Shell_NotifyIcon(NIM_ADD, &pData->notify)) {
			show_last_error(L"Shell_NotifyIcon: NIM_ADD");
		}
		break;
	}

	case WM_FOO_NOTIFY:
	{		
		int notification = LOWORD(lparam);
		switch (notification)
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