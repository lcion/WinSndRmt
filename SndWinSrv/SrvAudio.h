#pragma once
#include "Epvolume.h"

class CSrvAudio
{
public:
	CSrvAudio();
	~CSrvAudio();
	HRESULT Initialize();
	HRESULT SetMasterVolumeLevel(int vol);

private:
	IMMDevice *pDevice;
	IMMDeviceEnumerator *pEnumerator;
	CAudioEndpointVolumeCallback EPVolEvents;
	IAudioEndpointVolume *g_pEndptVol;
};
