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
#include "HotkeyRegistration.hxx"
#include "common.hxx"

HotkeyRegistration::HotkeyRegistration(HWND hwnd, Hotkeys hk, UINT mods, UINT vk)
	: _hwnd(hwnd)
	, _hk(hk)
	, _mods(mods)
	, _vk(vk)
{
	_registered = ::RegisterHotKey(hwnd, hk, mods, vk);
	if (!_registered) 
	{
		std::stringstream ss;
		ss << "Unalbe to register " << this->hotkey_str();
		throw std::runtime_error(ss.str().c_str());
	}	
}


HotkeyRegistration::HotkeyRegistration(Hotkeys hk, UINT mods, UINT vk)
	: _hk(hk)
	, _mods(mods)
	, _vk(vk)
{
	_registered = ::RegisterHotKey(NULL, hk, mods, vk);
	if (!_registered)
	{
		std::stringstream ss;
		ss << "Unalbe to register " << this->hotkey_str();
		throw std::runtime_error(ss.str().c_str());
	}
	::log_message(std::string("registration of ") + this->hotkey_str() + " successful");
}

HotkeyRegistration::~HotkeyRegistration()
{
	if (_registered) 
	{
		::UnregisterHotKey(_hwnd, _hk);
		::log_message(std::string("unregistration of ") + this->hotkey_str() + " successful");
	}
}

const char * HotkeyRegistration::hotkey_str()
{
	return ::hotkey_to_str(_hk);
}

const char * hotkey_to_str(Hotkeys hk)
{
	switch (hk)
	{
	case Hotkey_ToggleVolume:
		return "Hotkey_ToggleVolume";
	case Hotkey_VolumeDown:
		return "Hotkey_VolumeDown";
	case Hotkey_VolumeUp:
		return "Hotkey_VolumeUp";
	case Hotkey_Quit:
		return "Hotkey_Quit";
	default:
		return "Unrecognized Hotkeys enum member";
	}
}

