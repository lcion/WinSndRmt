#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

// Need to link with Ws2_32.lib
#pragma comment(lib, "ws2_32.lib")
#define DATA_BUFSIZE 4096

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    HRESULT hr = S_OK;
	char outTextBuff[512];
    char buffer[DATA_BUFSIZE];
    DWORD RecvBytes = 0;
    DWORD Flags = 0;
	DWORD Index = 0;
	BOOL bResult = TRUE;
    DWORD BytesTransferred = 0;
	char *ipAddress, *volValue, *next_token;

	int argn = 0;
	char *token = strtok_s(lpCmdLine, " ", &next_token);
	while(token){
		//use the token
		if(argn == 0) ipAddress = token;
		else if(argn == 1) volValue = token;
		else if(argn == 2) break; 
		argn++;
		token = strtok_s(NULL, " ", &next_token);
	}
	//expecting arguments ex: 192.168.1.12 10
	if(argn < 2) return 1;

    //----------------------
    // Initialize Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        //wprintf(L"WSAStartup function failed with error: %d\n", iResult);
		OutputDebugString("WSAStartup function failed with error: \n");
        return 1;
    }
    //----------------------
    // Create a SOCKET for connecting to server
    SOCKET ConnectSocket;
    ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ConnectSocket == INVALID_SOCKET) {
        // wprintf(L"socket function failed with error: %ld\n", WSAGetLastError());
		OutputDebugString("socket function failed with error: %ld\n");
        WSACleanup();
        return 1;
    }
    //----------------------
    // The sockaddr_in structure specifies the address family,
    // IP address, and port of the server to be connected to.
    sockaddr_in clientService;
    clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = inet_addr(ipAddress);
    clientService.sin_port = htons(27015);

    //----------------------
    // Connect to server.
    iResult = connect(ConnectSocket, (SOCKADDR *) & clientService, sizeof (clientService));
    if (iResult == SOCKET_ERROR) {
		OutputDebugString("connect function failed with error: %ld\n");
        //wprintf(L"connect function failed with error: %ld\n", WSAGetLastError());
        iResult = closesocket(ConnectSocket);
        if (iResult == SOCKET_ERROR)
			OutputDebugString("closesocket function failed with error: %ld\n");
            //wprintf(L"closesocket function failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    //wprintf(L"Connected to server.\n");
	OutputDebugString("Connected to server.\n");


	//setup overlapped read from socket
	//-----------------------------------------
    // Call WSARecv to receive data into DataBuf on 
    // the accepted socket in overlapped I/O mode
	WSAOVERLAPPED AcceptOverlapped;
    WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
    DWORD EventTotal = 0;
    WSABUF DataBuf;

    //-----------------------------------------
    // Create an event handle and setup an overlapped structure.
    EventArray[EventTotal] = WSACreateEvent();
    if (EventArray[EventTotal] == WSA_INVALID_EVENT) {
        OutputDebugString("WSACreateEvent failed with error\n");
		//wprintf(L"WSACreateEvent failed with error = %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

	ZeroMemory(&AcceptOverlapped, sizeof (WSAOVERLAPPED));
    AcceptOverlapped.hEvent = EventArray[EventTotal];

	DataBuf.len = DATA_BUFSIZE;
    DataBuf.buf = buffer;

    EventTotal++;

	do{
	// send some data
        //-----------------------------------------
        // set the volume on the server to 50%
		sprintf_s(buffer, "%s\n", volValue);
		DataBuf.len = strlen(buffer)+1;
        iResult =
            WSASend(ConnectSocket, &DataBuf, 1, &RecvBytes, Flags, &AcceptOverlapped, NULL);
        if (iResult != 0) {
			OutputDebugString("WSASend failed with error = \n");
            //wprintf(L"WSASend failed with error = %d\n", WSAGetLastError());
        }

        //-----------------------------------------         
        // Reset the changed flags and overlapped structure
        Flags = 0;
        ZeroMemory(&AcceptOverlapped, sizeof (WSAOVERLAPPED));

        AcceptOverlapped.hEvent = EventArray[Index - WSA_WAIT_EVENT_0];

        //-----------------------------------------
        // Reset the data buffer
        DataBuf.len = DATA_BUFSIZE;
        DataBuf.buf = buffer;

		if (WSARecv(ConnectSocket, &DataBuf, 1, &RecvBytes, &Flags, &AcceptOverlapped, NULL) ==
			SOCKET_ERROR) {
			iResult = WSAGetLastError();
			if (iResult != WSA_IO_PENDING){
				OutputDebugString("WSARecv failed with error \n");
				//wprintf(L"WSARecv failed with error = %d\n", iResult);
			}
		}
		// receive some data
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
        //-----------------------------------------
        // Determine the status of the overlapped event
        bResult =
            WSAGetOverlappedResult(ConnectSocket, &AcceptOverlapped, &BytesTransferred, FALSE,
                                   &Flags);
        if (bResult == FALSE) {
			OutputDebugString("WSAGetOverlappedResult failed with error ");
            //wprintf(L"WSAGetOverlappedResult failed with error = %d\n", WSAGetLastError());
        }
        //-----------------------------------------
        // If the connection has been closed, close the accepted socket
        if (BytesTransferred == 0) {
            sprintf_s(outTextBuff, "The connection has been closed, closing accept Socket %d\n", ConnectSocket);
			OutputDebugString(outTextBuff);
            closesocket(ConnectSocket);
            WSACloseEvent(EventArray[Index - WSA_WAIT_EVENT_0]);
            WSACleanup();
            return 1;
        }
		sprintf_s(outTextBuff, "The data received from server OK %d, %d\n", BytesTransferred, DataBuf.len);
		OutputDebugString(outTextBuff);
	}while(0);
    iResult = closesocket(ConnectSocket);
    if (iResult == SOCKET_ERROR) {
        //wprintf(L"closesocket function failed with error: %ld\n", WSAGetLastError());
		OutputDebugString("closesocket function failed with error: %ld\n");
        WSACleanup();
        return 1;
    }

    WSACleanup();
    return 0;
}
