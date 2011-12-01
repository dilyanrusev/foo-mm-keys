#include "stdafx.h"
#include "Resource.hxx"
#include "HotkeyRegistration.hxx"
#include "ComInit.hxx"
#include "AudioEndpointHandle.hxx"
#include "common.hxx"

struct MainWindowData {
	MainWindowData() 
		: pAudio(NULL) {
	}

	AudioEndpointHandle *pAudio;
};

///////////////////////////////////////////////////////////////////////////////
// Functions
///////////////////////////////////////////////////////////////////////////////
int run_message_loop();
void handle_hotkey(Hotkeys hotkey);
HWND create_main_window(HINSTANCE hInst, int show, MainWindowData *pAudio);
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
		
		HWND hwnd = create_main_window(hInst, nCmdShow, &data);

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
	
	case WM_HOTKEY:
		if (NULL != pData) {
			handle_hotkey((Hotkeys)wparam, pData->pAudio);
		}
		break;

	case WM_CLOSE:
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
// Function create_main_window(HINSTANCE, int, MainWindowData*)
///////////////////////////////////////////////////////////////////////////////

HWND create_main_window(HINSTANCE hInst, int show, MainWindowData *pData) {
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
	wc.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClassEx(&wc)) {
		throw std::runtime_error("RegisterClassEx failed");
	}

	HWND hwnd = CreateWindow(L"FooMMKeys", L"Foo Multimedia Keys",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		HWND_DESKTOP, NULL, hInst, pData);
	if (NULL == hwnd) {
		throw std::runtime_error("CreateWindow failed");
	}

	ShowWindow(hwnd, show);
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