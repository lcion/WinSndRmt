//used to handle the network communications
#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include "SrvNetwrk.h"

CSrvNetwrk::CSrvNetwrk(CSrvAudio &cSrvAudio, CSrvApp &cSrvApp)
	: volAudio(cSrvAudio), myAppLogic(cSrvApp)
{
	ListenSocket = INVALID_SOCKET;
    AcceptSocket = INVALID_SOCKET;
    RecvBytes = 0;
    Flags = 0;
    BytesTransferred = 0;

	ZeroMemory( &wsaData, sizeof(wsaData) );
}

HRESULT CSrvNetwrk::Initialize(){
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

	u_short port = 27015;
    char *ip;
    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_port = htons(port);
    hostent *thisHost;

    thisHost = gethostbyname("");
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
    sprintf_s(outTextBuff, "Listening on port %d\n", port);
	OutputDebugString(outTextBuff);

	ZeroMemory(&ReadOverlapped, sizeof (WSAOVERLAPPED));
    ZeroMemory(&WriteOverlapped, sizeof (WSAOVERLAPPED));

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
    OutputDebugString("Client Accepted...\n");
	return hr;
}

CSrvNetwrk::~CSrvNetwrk()
{
	if(ReadOverlapped.hEvent){
		WSACloseEvent(ReadOverlapped.hEvent);
		ReadOverlapped.hEvent = NULL;
	}
	if(WriteOverlapped.hEvent){
		WSACloseEvent(WriteOverlapped.hEvent);
		WriteOverlapped.hEvent = NULL;
	}
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
	sprintf_s(outTextBuff, "The data received, send it back %d, %d\n", BytesTransferred, DataBuf.len);
	OutputDebugString(outTextBuff);
	DataBuf.len = BytesTransferred;
	int vol = myAppLogic.ProcessClient(DataBuf.buf, DataBuf.len);
	if(vol > -1){
		hr = volAudio.SetMasterVolumeLevel(vol);
		if(FAILED(hr)) return -1;
	}
	//-----------------------------------------
	// If data has been received, echo the received data
	// from DataBuf back to the client
	iResult =
		WSASend(AcceptSocket, &DataBuf, 1, &RecvBytes, Flags, &WriteOverlapped, NULL);
	if (iResult != 0) {
		OutputDebugString("WSASend failed with error = \n");
	}
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

	ReceiveAsync();
}

//called from different thread from the audio events 
void CSrvNetwrk::SendDataFromAudioEvents(int volume){
	//if client is connected send data
	if(AcceptSocket != INVALID_SOCKET){
		InterlockedExchange((LONG *)&iVolume, (LONG )volume);
		WSASetEvent(audioEventH);
	}
}

void CSrvNetwrk::OnSndEvent(){
		sprintf_s(buffer, "%d\n", iVolume);
		DataBuf.len = strlen(buffer)+1;

		int iResult =
			WSASend(AcceptSocket, &DataBuf, 1, &RecvBytes, Flags, &WriteOverlapped, NULL);
		if (iResult != 0) {
			OutputDebugString("WSASend failed with error = \n");
		}
}
