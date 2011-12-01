// NativeHotKeys.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "HotkeyRegistration.hxx"
#include "ComInit.hxx"
#include "AudioEndpointHandle.hxx"
#include "common.hxx"

int run_app();

int _tmain(int argc, _TCHAR* argv[])
{
	try {
		return run_app();
	}
	catch (const std::exception &ex) {
		fputs(ex.what(), stderr);
		return 1;
	}
}

int run_app() 
{
	HotkeyRegistration hkToggle(Hotkey_ToggleVolume, MOD_ALT | MOD_CONTROL, VK_F10);
	HotkeyRegistration hkDown(Hotkey_VolumeDown, MOD_ALT | MOD_CONTROL, VK_F11);
	HotkeyRegistration hkUp(Hotkey_VolumeUp, MOD_ALT | MOD_CONTROL, VK_F12);
	HotkeyRegistration hkQuit(Hotkey_Quit, MOD_ALT | MOD_CONTROL, VK_BACK);
	ComInit comInit;
	AudioEndpointHandle audioHandle;	
		
	//CoCreateInstance(refclas, NULL, CLSCTX_LOCAL_SERVER, riid, 

	log_message("Press Ctrl+Alt+Backspace to quit");
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		if (msg.message == WM_HOTKEY)
		{
			Hotkeys hk = (Hotkeys)msg.wParam;			
			log_message(std::string(hotkey_to_str(hk)) + " was pressed");
			switch (hk)
			{
			case Hotkey_Quit:
				PostQuitMessage(0);
				break;

			case Hotkey_ToggleVolume:
				audioHandle.toggle_mute();
				break;

			case Hotkey_VolumeDown:
				audioHandle.decrease_volume();
				break;

			case Hotkey_VolumeUp:
				audioHandle.increase_volume();
				break;

			default:
				log_message("Not implemented");
				break;
			}
		}		
	}

	return (int)msg.wParam;
}