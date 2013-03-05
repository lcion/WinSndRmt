//application entry point
//command line pharsing
#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>

#include "SrvApp.h"
#include "SrvNetwrk.h"
#include "SrvAudio.h"

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine,
                     int nCmdShow)
{
	CSrvAudio volAudio;
    HRESULT hr = S_OK;

	hr = volAudio.Initialize();
	if(FAILED(hr)) return -1;

    //-----------------------------------------
    // Declare and initialize variables
    WSADATA wsaData = { 0 };
    int iResult = 0;
    BOOL bResult = TRUE;

    WSABUF DataBuf;
    char buffer[DATA_BUFSIZE];
	char outTextBuff[512];

    DWORD EventTotal = 0;
    DWORD RecvBytes = 0;
    DWORD Flags = 0;
    DWORD BytesTransferred = 0;

    WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
    WSAOVERLAPPED ReadOverlapped;
    WSAOVERLAPPED WriteOverlapped;
    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET AcceptSocket = INVALID_SOCKET;

    DWORD Index;
	CSrvApp myAppLogic;

    if (hPrevInstance)
    {
        return 0;
    }

    //-----------------------------------------
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        OutputDebugString("WSAStartup failed: \n");
		//wprintf(L"WSAStartup failed: %d\n", iResult);
        return 1;
    }
    //-----------------------------------------
    // Create a listening socket bound to a local
    // IP address and the port specified
    ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ListenSocket == INVALID_SOCKET) {
        OutputDebugString("socket failed with error = \n");
		//wprintf(L"socket failed with error = %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    u_short port = 27015;
    char *ip;
    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_port = htons(port);
    hostent *thisHost;

    thisHost = gethostbyname("");
    if (thisHost == NULL) {
        OutputDebugString("gethostbyname failed with error = \n");
		//wprintf(L"gethostbyname failed with error = %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    ip = inet_ntoa(*(struct in_addr *) *thisHost->h_addr_list);

    service.sin_addr.s_addr = inet_addr(ip);

    //-----------------------------------------
    // Bind the listening socket to the local IP address
    // and port number
    iResult = bind(ListenSocket, (SOCKADDR *) & service, sizeof (SOCKADDR));
    if (iResult != 0) {
        OutputDebugString("bind failed with error = %d\n");
		//wprintf(L"bind failed with error = %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    //-----------------------------------------
    // Set the socket to listen for incoming
    // connection requests
    iResult = listen(ListenSocket, 1);
    if (iResult != 0) {
        OutputDebugString("listen failed with error = \n");
		//wprintf(L"listen failed with error = %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    sprintf_s(outTextBuff, "Listening on port %d\n", port);
	OutputDebugString(outTextBuff);

    //-----------------------------------------
    // Accept and incoming connection request
    AcceptSocket = accept(ListenSocket, NULL, NULL);
    if (AcceptSocket == INVALID_SOCKET) {
        OutputDebugString("accept failed with error = \n");
		//wprintf(L"accept failed with error = %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    OutputDebugString("Client Accepted...\n");
	//wprintf(L"Client Accepted...\n");

    //-----------------------------------------
    // Create an event handle and setup an overlapped structure for read.
    EventArray[EventTotal] = WSACreateEvent();
    if (EventArray[EventTotal] == WSA_INVALID_EVENT) {
        OutputDebugString("WSACreateEvent failed with error\n");
		//wprintf(L"WSACreateEvent failed with error = %d\n", WSAGetLastError());
        closesocket(AcceptSocket);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    ZeroMemory(&ReadOverlapped, sizeof (WSAOVERLAPPED));
    ReadOverlapped.hEvent = EventArray[EventTotal];

    DataBuf.len = DATA_BUFSIZE;
    DataBuf.buf = buffer;

    EventTotal++;
    //-----------------------------------------
    // Create an event handle and setup an overlapped structure for write.
    EventArray[EventTotal] = WSACreateEvent();
    if (EventArray[EventTotal] == WSA_INVALID_EVENT) {
        OutputDebugString("WSACreateEvent failed with error\n");
		//wprintf(L"WSACreateEvent failed with error = %d\n", WSAGetLastError());
        closesocket(AcceptSocket);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    ZeroMemory(&WriteOverlapped, sizeof (WSAOVERLAPPED));
    WriteOverlapped.hEvent = EventArray[EventTotal];

    DataBuf.len = DATA_BUFSIZE;
    DataBuf.buf = buffer;

    EventTotal++;

    //-----------------------------------------
    // Call WSARecv to receive data into DataBuf on 
    // the accepted socket in overlapped I/O mode
    if (WSARecv(AcceptSocket, &DataBuf, 1, &RecvBytes, &Flags, &ReadOverlapped, NULL) ==
        SOCKET_ERROR) {
        iResult = WSAGetLastError();
        if (iResult != WSA_IO_PENDING){
			OutputDebugString("WSARecv failed with error \n");
            //wprintf(L"WSARecv failed with error = %d\n", iResult);
		}
    }
    //-----------------------------------------
    // Process overlapped receives on the socket
    while (1) {

        //-----------------------------------------
        // Wait for the overlapped I/O call to complete
        Index = WSAWaitForMultipleEvents(EventTotal, EventArray, FALSE, WSA_INFINITE, FALSE);

        //-----------------------------------------
        // Reset the signaled event
        bResult = WSAResetEvent(EventArray[Index - WSA_WAIT_EVENT_0]);
        if (bResult == FALSE) {
			OutputDebugString("WSAResetEvent failed with error \n");
            //wprintf(L"WSAResetEvent failed with error = %d\n", WSAGetLastError());
        }
		//if read operation completed
		if((Index - WSA_WAIT_EVENT_0) == 0){
			//-----------------------------------------
			// Determine the status of the overlapped event
			bResult =
				WSAGetOverlappedResult(AcceptSocket, &ReadOverlapped, &BytesTransferred, FALSE,
									   &Flags);
			if (bResult == FALSE) {
				OutputDebugString("WSAGetOverlappedResult failed with error on ReadOverlapped");
				//wprintf(L"WSAGetOverlappedResult failed with error = %d\n", WSAGetLastError());
			}
			//-----------------------------------------
			// If the connection has been closed, close the accepted socket
			if (BytesTransferred == 0) {
				sprintf_s(outTextBuff, "The connection has been closed, closing accept Socket %d\n", AcceptSocket);
				OutputDebugString(outTextBuff);
				closesocket(ListenSocket);
				closesocket(AcceptSocket);
				WSACloseEvent(EventArray[Index - WSA_WAIT_EVENT_0]);
				WSACleanup();
				goto Exit;
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
				//wprintf(L"WSASend failed with error = %d\n", WSAGetLastError());
			}
		}else
		{
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
			// Call WSARecv to receive data into DataBuf on
			// the accepted socket in overlapped I/O mode
			if (WSARecv(AcceptSocket, &DataBuf, 1, &RecvBytes, &Flags, &ReadOverlapped, NULL) ==
				SOCKET_ERROR) {
				iResult = WSAGetLastError();
				if (iResult != WSA_IO_PENDING){
					OutputDebugString("WSARecv failed with error \n");
					//wprintf(L"WSARecv failed with error = %d\n", iResult);
				}
			}
		}

    }

Exit:
    closesocket(ListenSocket);
    closesocket(AcceptSocket);
    WSACleanup();

    return 0;
}
