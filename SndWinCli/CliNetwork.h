#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#define DATA_BUFSIZE 4096

class CCliNetwork
{
public:
	CCliNetwork(void);
	~CCliNetwork(void);

	int Initialize();
	int Connect(char *ipAddress);
	void DoTimerProc();
	void SendVolume(int vol);

private:
	void OnDataReceived();
	void OnDataSent();
	void ReceiveVolume();

	SOCKET ConnectSocket;
	WSAOVERLAPPED ReadOverlapped;
	WSAOVERLAPPED WriteOverlapped;
	WSABUF DataBuf;
	char buffer[DATA_BUFSIZE];
	WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
	DWORD EventTotal;
	DWORD RecvBytes;
	DWORD Flags;

};

