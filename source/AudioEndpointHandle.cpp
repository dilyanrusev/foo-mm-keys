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

#include "StdAfx.h"
#include "AudioEndpointHandle.hxx"
#include "common.hxx"

AudioEndpointHandle::AudioEndpointHandle(void)	
	: _pDevEnumerator(NULL)
	, _pAudioDeveice(NULL)
	, _pEndpointVolume(NULL)
{
	HRESULT hr;
	
	log_message("Getting IMMDeviceEnumerator...");
	hr = ::CoCreateInstance(
		__uuidof(MMDeviceEnumerator), 
		NULL, 
		CLSCTX_ALL, 
		__uuidof(IMMDeviceEnumerator), 
		(void**)&_pDevEnumerator
	);

	if (FAILED(hr)) {
		throw std::runtime_error("Failed to obtain instance of IMMDeviceEnumerator");
	}

	log_message("Getting IMMDevice...");
	hr = _pDevEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &_pAudioDeveice);
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to obtain instance of IMMDevice");
	}

	log_message("Getting IAudioEndpointVolume...");
	hr = _pAudioDeveice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_LOCAL_SERVER, NULL, (void**)&_pEndpointVolume);
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to obtain instance of IAudioEndpointVolume");
	}
	
	hr = _pEndpointVolume->GetVolumeRange(&_minVolume, &_maxVolume, &_volumeIncrement);
	if (FAILED(hr)) {
		throw std::runtime_error("Unable to determine device volume range");
	}
}


AudioEndpointHandle::~AudioEndpointHandle(void)
{
	if (NULL != _pEndpointVolume) {
		_pEndpointVolume->Release();
		_pEndpointVolume = NULL;
		::log_message("IAudioEndpointVolume released");
	}
	if (NULL != _pAudioDeveice) {
		_pAudioDeveice->Release();
		_pAudioDeveice = NULL;
		::log_message("IMMDevice released");
	}
	if (NULL != _pDevEnumerator) {
		_pDevEnumerator->Release();
		_pDevEnumerator = NULL;
		::log_message("IMMDeviceEnumerator released");
	}
}

void AudioEndpointHandle::toggle_mute() {	
	BOOL muted;
	if (SUCCEEDED(_pEndpointVolume->GetMute(&muted))) {
		if (FAILED(_pEndpointVolume->SetMute(!muted, NULL))) {
			::log_message("IAudioEndpointVolume::SetMute failed");
		}
	}	
	else {
		::log_message("IAudioEndpointVolume::GetMute failed");
	}
}

void AudioEndpointHandle::increase_volume() {
	float currentVolume;
	if (SUCCEEDED(_pEndpointVolume->GetMasterVolumeLevel(&currentVolume))) {
		float newVolumeLevel = min(currentVolume + _volumeIncrement, _maxVolume);
		if (FAILED(_pEndpointVolume->SetMasterVolumeLevel(newVolumeLevel, NULL))) {
			::log_message("IAudioEndpointVolume::SetMute failed");
		}
	}	
	else {
		::log_message("IAudioEndpointVolume::GetMasterVolumeLevel failed");
	}
}

void AudioEndpointHandle::decrease_volume() {
	float currentVolume;
	if (SUCCEEDED(_pEndpointVolume->GetMasterVolumeLevel(&currentVolume))) {
		float newVolumeLevel = max(currentVolume - _volumeIncrement, _minVolume);
		if (FAILED(_pEndpointVolume->SetMasterVolumeLevel(newVolumeLevel, NULL))) {
			::log_message("IAudioEndpointVolume::SetMute failed");
		}
	}	
	else {
		::log_message("IAudioEndpointVolume::GetMasterVolumeLevel failed");
	}
}
