//used to handle the audio part of the application
#include <stdio.h>
#include "SrvAudio.h"

GUID g_guidMyContext = GUID_NULL;

CSrvAudio::CSrvAudio()
{
	g_pEndptVol = NULL;
}

HRESULT CSrvAudio::Initialize(void *ntwrkCls){
	HRESULT hr = S_OK;

	CoInitialize(NULL);

    hr = CoCreateGuid(&g_guidMyContext);
	if(FAILED(hr)) return hr;

	//snd specific initialization
	pEnumerator = NULL;
    pDevice = NULL;

	// Get enumerator for audio endpoint devices.
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),
                          NULL, CLSCTX_INPROC_SERVER,
                          __uuidof(IMMDeviceEnumerator),
                          (void**)&pEnumerator);
	if(FAILED(hr)) return hr;

	// Get default audio-rendering device.
    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);

	hr = pDevice->Activate(__uuidof(IAudioEndpointVolume),
                           CLSCTX_ALL, NULL, (void**)&g_pEndptVol);
	if(FAILED(hr)) return hr;

	hr = g_pEndptVol->RegisterControlChangeNotify(
                     (IAudioEndpointVolumeCallback*)&EPVolEvents);
	if(FAILED(hr)) return hr;
	EPVolEvents.setNtwrkRef(ntwrkCls);
	return hr;
}

HRESULT CSrvAudio::SetMasterVolumeLevel(int vol){
	HRESULT hr = S_OK;
	float fVolume = (float)vol/MAX_VOL;
	hr = g_pEndptVol->SetMasterVolumeLevelScalar(fVolume, &g_guidMyContext);

	return hr;
}

CSrvAudio::~CSrvAudio()
{
    if (pEnumerator != NULL)
    {
        g_pEndptVol->UnregisterControlChangeNotify(
                    (IAudioEndpointVolumeCallback*)&EPVolEvents);
    }

    SAFE_RELEASE(pEnumerator)
    SAFE_RELEASE(pDevice)
    SAFE_RELEASE(g_pEndptVol)
    CoUninitialize();
}

//class CSrvNetwrk;
#include "SrvNetwrkInterface.h"
void CAudioEndpointVolumeCallback::sndVolumeOnNetwork(int volume){
	if(ntwrkClsRef){
		CSrvNetwrkInterface *srvNetwrk = (CSrvNetwrkInterface*)ntwrkClsRef;
		srvNetwrk->SendDataFromAudioEvents(volume);
	}
}