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

