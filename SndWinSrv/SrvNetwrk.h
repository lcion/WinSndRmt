#pragma once
#include <windows.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include "SrvAudio.h"
#include "SrvApp.h"
#include "SrvNetwrkInterface.h"

#define DATA_BUFSIZE 4096

class CSrvNetwrk : public CSrvNetwrkInterface
{
public:
	CSrvNetwrk(CSrvAudio &cSrvAudio, CSrvApp &cSrvApp);
	~CSrvNetwrk();
	HRESULT Initialize();
	HRESULT Accept();
	void SetReadEvent(HANDLE hEvent);
	void SetWriteEvent(HANDLE hEvent);
	void ReceiveAsync();
	int OnDataReceived();
	void OnDataSent();
	void CloseClientSocket();
	void SendDataFromAudioEvents(int volume, BOOL bMute);
	void OnSndEvent();
	void SetAudioEvent(HANDLE hEvent);

private:
	WSADATA wsaData;
	SOCKET ListenSocket;
    SOCKET AcceptSocket;
    WSAOVERLAPPED ReadOverlapped;
    WSAOVERLAPPED WriteOverlapped;
    WSABUF DataBuf;
    char buffer[DATA_BUFSIZE];
    DWORD RecvBytes;
    DWORD Flags;
    DWORD BytesTransferred;
	char outTextBuff[512];
	CSrvAudio &volAudio;
	CSrvApp &myAppLogic;
	int iVolume;
	BOOL mbMute;
	HANDLE audioEventH;
};
