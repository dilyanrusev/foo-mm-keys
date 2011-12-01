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
