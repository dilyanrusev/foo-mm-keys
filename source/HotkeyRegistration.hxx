#pragma once

#include <Windows.h>
#include <vector>

enum Hotkeys {
	Hotkey_ToggleVolume,
	Hotkey_VolumeUp,
	Hotkey_VolumeDown,
	Hotkey_Quit
};

const char * hotkey_to_str(Hotkeys hk);

class HotkeyRegistration {
	Hotkeys _hk;
	HWND _hwnd;
	UINT _mods;
	UINT _vk;
	BOOL _registered;

public:
	HotkeyRegistration(HWND hwnd, Hotkeys hk, UINT mods, UINT vk);
	HotkeyRegistration(Hotkeys hk, UINT mods, UINT vk);
	virtual ~HotkeyRegistration();

	const char * hotkey_str();
};
