#pragma once

#include <Mmdeviceapi.h>
#include <Endpointvolume.h>

class AudioEndpointHandle
{	
	IMMDeviceEnumerator *_pDevEnumerator;
	IMMDevice *_pAudioDeveice;
	IAudioEndpointVolume *_pEndpointVolume;
	
	float _minVolume;
	float _maxVolume;
	float _volumeIncrement;

public:
	AudioEndpointHandle(void);
	virtual ~AudioEndpointHandle(void);

	void toggle_mute();
	void increase_volume();
	void decrease_volume();	
};

