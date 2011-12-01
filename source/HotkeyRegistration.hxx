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
