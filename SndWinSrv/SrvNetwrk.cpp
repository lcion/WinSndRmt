//used to handle the network communications
#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include "SrvNetwrk.h"

CSrvNetwrk::CSrvNetwrk(CSrvAudio &cSrvAudio, CSrvApp &cSrvApp)
	: volAudio(cSrvAudio), myAppLogic(cSrvApp), audioEventH(NULL)
{
	ListenSocket = INVALID_SOCKET;
    AcceptSocket = INVALID_SOCKET;
    RecvBytes = 0;
    Flags = 0;
    BytesTransferred = 0;

	ZeroMemory( &wsaData, sizeof(wsaData) );
	ZeroMemory(&ReadOverlapped, sizeof (WSAOVERLAPPED));
    ZeroMemory(&WriteOverlapped, sizeof (WSAOVERLAPPED));
}

HRESULT CSrvNetwrk::Initialize(char *hostName, int port){
	HRESULT hr = S_OK;
	int iResult = 0;

    //-----------------------------------------
    // Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) return -1;

	//-----------------------------------------
    // Create a listening socket bound to a local
    // IP address and the port specified
    ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ListenSocket == INVALID_SOCKET) return -1;

	u_short uPort = port;
    char *ip;
    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_port = htons(uPort);
    hostent *thisHost;

    thisHost = gethostbyname(hostName);
    if (thisHost == NULL) {
        OutputDebugString("gethostbyname failed with error = \n");
        return -1;
    }

    ip = inet_ntoa(*(struct in_addr *) *thisHost->h_addr_list);

    service.sin_addr.s_addr = inet_addr(ip);

    //-----------------------------------------
    // Bind the listening socket to the local IP address
    // and port number
    iResult = bind(ListenSocket, (SOCKADDR *) & service, sizeof (SOCKADDR));
    if (iResult != 0) {
        OutputDebugString("bind failed with error = %d\n");
        return -1;
    }
    //-----------------------------------------
    // Set the socket to listen for incoming
    // connection requests
    iResult = listen(ListenSocket, 1);
    if (iResult != 0) {
        OutputDebugString("listen failed with error = \n");
        return -1;
    }
    sprintf_s(outTextBuff, "Listening on ip %s port %d\n", ip, uPort);
	OutputDebugString(outTextBuff);

    DataBuf.len = DATA_BUFSIZE;
    DataBuf.buf = buffer;

	return hr;
}

void CSrvNetwrk::CloseClientSocket()
{
	if(AcceptSocket != INVALID_SOCKET){
		closesocket(AcceptSocket);
		AcceptSocket = INVALID_SOCKET;
	}
}

HRESULT CSrvNetwrk::Accept()
{
	HRESULT hr = S_OK;
    //-----------------------------------------
    // Accept and incoming connection request
    AcceptSocket = accept(ListenSocket, NULL, NULL);
    if (AcceptSocket == INVALID_SOCKET) {
        OutputDebugString("accept failed with error = \n");
        return -1;
    }
	iVolume[1]=101;
	mbMute[1]=101;
    OutputDebugString("Client Accepted...\n");
	return hr;
}

CSrvNetwrk::~CSrvNetwrk()
{
	//close events
	if(ReadOverlapped.hEvent){
		WSACloseEvent(ReadOverlapped.hEvent);
		ReadOverlapped.hEvent = NULL;
	}
	if(WriteOverlapped.hEvent){
		WSACloseEvent(WriteOverlapped.hEvent);
		WriteOverlapped.hEvent = NULL;
	}
	if(audioEventH){
		WSACloseEvent(audioEventH);
		audioEventH = NULL;
	}

	//close sockets
	if(AcceptSocket != INVALID_SOCKET){
	    closesocket(AcceptSocket);
		AcceptSocket = INVALID_SOCKET;
	}
	if(ListenSocket != INVALID_SOCKET){
		closesocket(ListenSocket);
		ListenSocket = INVALID_SOCKET;
	}
	WSACleanup();
}

HRESULT CSrvNetwrk::CreateEvents(WSAEVENT *EventArray, DWORD &EventTotal){
	//-----------------------------------------
    // Create an event handle and setup an overlapped structure for read.
    EventArray[EventTotal] = WSACreateEvent();
    if (EventArray[EventTotal] == WSA_INVALID_EVENT) {
        OutputDebugString("WSACreateEvent failed with error\n");
        return 1;
    }

	SetReadEvent(EventArray[EventTotal]);

    EventTotal++;

    //-----------------------------------------
    // Create an event handle and setup an overlapped structure for write.
    EventArray[EventTotal] = WSACreateEvent();
    if (EventArray[EventTotal] == WSA_INVALID_EVENT) {
        OutputDebugString("WSACreateEvent failed with error\n");
        return 1;
    }

    //WriteOverlapped.hEvent = EventArray[EventTotal];
	SetWriteEvent(EventArray[EventTotal]);

    EventTotal++;

	//-----------------------------------------
    // Create an event handle for the audio events
    EventArray[EventTotal] = WSACreateEvent();
    if (EventArray[EventTotal] == WSA_INVALID_EVENT) {
        OutputDebugString("WSACreateEvent failed with error\n");
        return 1;
    }
	SetAudioEvent(EventArray[EventTotal]);
    EventTotal++;
	return 0;
}

void CSrvNetwrk::SetReadEvent(HANDLE hEvent)
{
	ReadOverlapped.hEvent = hEvent;
}

void CSrvNetwrk::SetWriteEvent(HANDLE hEvent)
{
	WriteOverlapped.hEvent = hEvent;
}

void CSrvNetwrk::SetAudioEvent(HANDLE hEvent)
{
	audioEventH = hEvent;
}

void CSrvNetwrk::ReceiveAsync(){
	int iResult = 0;
    //-----------------------------------------
    // Call WSARecv to receive data into DataBuf on 
    // the accepted socket in overlapped I/O mode
    if (WSARecv(AcceptSocket, &DataBuf, 1, &RecvBytes, &Flags, &ReadOverlapped, NULL) ==
        SOCKET_ERROR) {
        iResult = WSAGetLastError();
        if (iResult != WSA_IO_PENDING){
			OutputDebugString("WSARecv failed with error \n");
		}
    }

}

int CSrvNetwrk::OnDataReceived(){
	BOOL bResult = TRUE;
	int iResult = 0;
    HRESULT hr = S_OK;
	//-----------------------------------------
	// Determine the status of the overlapped event
	bResult =
		WSAGetOverlappedResult(AcceptSocket, &ReadOverlapped, &BytesTransferred, FALSE,
								&Flags);
	if (bResult == FALSE) {
		OutputDebugString("WSAGetOverlappedResult failed with error on ReadOverlapped");
	}
	//-----------------------------------------
	// If the connection has been closed, close the accepted socket
	if (BytesTransferred == 0) {
		sprintf_s(outTextBuff, "The connection has been closed, closing accept Socket %d\n", AcceptSocket);
		OutputDebugString(outTextBuff);
		return -1;
	}
	sprintf_s(outTextBuff, "The data received bytes[%d], bufferLen[%d], data[%s]\n", BytesTransferred, DataBuf.len, DataBuf.buf);
	OutputDebugString(outTextBuff);
	DataBuf.len = BytesTransferred;
	int cliResult[5] = {0,0,0,0,0};
	//process buffer sent by the client
	myAppLogic.ProcessClient(DataBuf.buf, DataBuf.len, &cliResult[0]);
	//execute the commands received
	if(cliResult[0] == 1){// we have volume set command
		hr = volAudio.SetMasterVolumeLevel(cliResult[1]);
		if(FAILED(hr)) return -1;
	}
	if(cliResult[2] == 1){// we have volume set command
		volAudio.SetMute(cliResult[3]);
	}
	if(cliResult[4] == 1){// we have volume set command
		LockWorkStation();
	}

	//setup async read to receive more data
	ReceiveAsync();
	return 0;
}

void CSrvNetwrk::OnDataSent()
{
	BOOL bResult = TRUE;
	//write overlapped did something
	// Determine the status of the overlapped event
	bResult =
		WSAGetOverlappedResult(AcceptSocket, &WriteOverlapped, &BytesTransferred, FALSE,
								&Flags);
	if (bResult == FALSE) {
		OutputDebugString("WSAGetOverlappedResult failed with error on ReadOverlapped");
		//wprintf(L"WSAGetOverlappedResult failed with error = %d\n", WSAGetLastError());
	}
	sprintf_s(outTextBuff, "The data is sent %d, %d\n", BytesTransferred, DataBuf.len);
	OutputDebugString(outTextBuff);

	DataBuf.len = DATA_BUFSIZE;
	DataBuf.buf = buffer;
	RecvBytes = 0;
}

//called from different thread from the audio events 
void CSrvNetwrk::SendDataFromAudioEvents(int volume, BOOL bMute){
	//if client is connected send data
	if(AcceptSocket != INVALID_SOCKET){
		InterlockedExchange((LONG *)&iVolume[0], (LONG )volume);
		InterlockedExchange((LONG *)&mbMute[0], (LONG )bMute);
		WSASetEvent(audioEventH);
	}
}

void CSrvNetwrk::OnSndEvent(){
	DataBuf.len = 0;
	if(iVolume[1] != iVolume[0]){
		iVolume[1] = iVolume[0];
		buffer[0] = 4;			// size of package for client
		buffer[1] = RMT_VOLUME;	// function
		buffer[2] = iVolume[0];	// arg0
		buffer[3] = 0;			// arg1
		DataBuf.len += 4;
	}
	if(mbMute[1] != mbMute[0]){
		mbMute[1] = mbMute[0];
		buffer[DataBuf.len+0] = 4;			// size of package for client
		buffer[DataBuf.len+1] = RMT_MUTE;	// function
		buffer[DataBuf.len+2] = mbMute[0];	// arg0
		buffer[DataBuf.len+3] = 0;			// arg1
		DataBuf.len += 4;
	}

	int iResult =
		WSASend(AcceptSocket, &DataBuf, 1, &RecvBytes, Flags, &WriteOverlapped, NULL);
	if (iResult != 0) {
		OutputDebugString("WSASend failed with error = \n");
	}
}
