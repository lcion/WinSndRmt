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
	int Connect(char *ipAddress, int port);
	void DoTimerProc();
	void SendVolume(int vol);
	void SendMute(BOOL mute);
	void SendLockCmd();
	void SendPauseCmd();

private:
	void OnDataReceived();
	void OnDataSent();
	void ReceiveVolume();
	void UpdateDialog(DWORD BytesTransferred);

	SOCKET ConnectSocket;
	WSAOVERLAPPED ReadOverlapped;
	WSAOVERLAPPED WriteOverlapped;
	WSABUF DataBuf;
	char buffer[DATA_BUFSIZE];
	WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
	DWORD EventTotal;
	DWORD RecvBytes;
	DWORD Flags;
	enum _func_enum {
		RMT_LOGIN,
		RMT_VOLUME,
		RMT_MUTE,
		RMT_LOCK,
		RMT_PAUSE,
		RMT_SLEEP,
		RMT_MOUSE_DOWN_UP,
		RMT_MOUSE_MOVE,
		RMT_LOGOUT
	} func_enum;
};

