#include <windows.h>
#include <stdio.h>

#include "..\SndWinSrv\srvaudio.h"

CSrvAudio
::CSrvAudio
()
{
}

CSrvAudio
::~CSrvAudio
()
{
}
GUID g_guidMyContext = GUID_NULL;

HRESULT CSrvAudio::SetMute(BOOL mute){
	HRESULT hr = S_OK;
	return hr;
}

HRESULT CSrvAudio::SetMasterVolumeLevel(int vol){
	HRESULT hr = S_OK;
	return hr;
}

void CAudioEndpointVolumeCallback::sndVolumeOnNetwork(int volume, BOOL bMute){
}
