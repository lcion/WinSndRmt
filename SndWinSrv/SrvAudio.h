#pragma once
#include "Epvolume.h"

class CSrvAudio
{
public:
	CSrvAudio();
	~CSrvAudio();
	HRESULT Initialize(void *ntwrkCls);
	HRESULT SetMasterVolumeLevel(int vol);
	void PostVolValToClient();

private:
	IMMDevice *pDevice;
	IMMDeviceEnumerator *pEnumerator;
	CAudioEndpointVolumeCallback EPVolEvents;
	IAudioEndpointVolume *g_pEndptVol;
	void *pNetwrkClass;
};
