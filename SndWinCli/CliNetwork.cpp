#include "CliNetwork.h"

CCliNetwork::CCliNetwork(void): ConnectSocket(INVALID_SOCKET), RecvBytes(0), Flags(0)
{
	ZeroMemory(&ReadOverlapped, sizeof (WSAOVERLAPPED));
	ZeroMemory(&WriteOverlapped, sizeof (WSAOVERLAPPED));
	
	DataBuf.len = DATA_BUFSIZE;
    DataBuf.buf = buffer;
}

CCliNetwork::~CCliNetwork(void)
{
	if(ReadOverlapped.hEvent){
		WSACloseEvent(ReadOverlapped.hEvent);
		ReadOverlapped.hEvent = NULL;
	}
	if(WriteOverlapped.hEvent){
		WSACloseEvent(WriteOverlapped.hEvent);
		WriteOverlapped.hEvent = NULL;
	}
	
	if (ConnectSocket != INVALID_SOCKET){
        if (closesocket(ConnectSocket) == SOCKET_ERROR)
			OutputDebugString("closesocket function failed with error: %ld\n");
		ConnectSocket = INVALID_SOCKET;
	}
	WSACleanup();
}

int CCliNetwork::Initialize()
{
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
    ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ConnectSocket == INVALID_SOCKET) {
        // wprintf(L"socket function failed with error: %ld\n", WSAGetLastError());
		OutputDebugString("socket function failed with error: %ld\n");
        return 1;
    }

	//setup overlapped read from socket
	//-----------------------------------------
    // Call WSARecv to receive data into DataBuf on 
    // the accepted socket in overlapped I/O mode
    EventTotal = 0;

    //-----------------------------------------
    // Create an event handle and setup an overlapped structure for async reading.
    EventArray[EventTotal] = WSACreateEvent();
    if (EventArray[EventTotal] == WSA_INVALID_EVENT) {
        OutputDebugString("WSACreateEvent failed with error\n");
		//wprintf(L"WSACreateEvent failed with error = %d\n", WSAGetLastError());
        return 1;
    }

    ReadOverlapped.hEvent = EventArray[EventTotal];

    EventTotal++;

    //-----------------------------------------
    // Create an event handle and setup an overlapped structure for async writing.
    EventArray[EventTotal] = WSACreateEvent();
    if (EventArray[EventTotal] == WSA_INVALID_EVENT) {
        OutputDebugString("WSACreateEvent failed with error\n");
		//wprintf(L"WSACreateEvent failed with error = %d\n", WSAGetLastError());
        return 1;
    }

    WriteOverlapped.hEvent = EventArray[EventTotal];

    EventTotal++;
	return 0;
}

int CCliNetwork::Connect(char *ipAddress)
{
	int iResult;
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
        return 1;
    }
	ReceiveVolume();
	return 0;
}

//check for read or write complete events from the socket
void CCliNetwork::DoTimerProc()
{
	//-----------------------------------------
    // Wait for the overlapped I/O call to complete
    int Index = WSAWaitForMultipleEvents(EventTotal, EventArray, FALSE, FALSE, FALSE);

	//there is nothing to process
	if(Index == WSA_WAIT_TIMEOUT) return;

	//-----------------------------------------
    // Reset the signaled event
    BOOL bResult = WSAResetEvent(EventArray[Index - WSA_WAIT_EVENT_0]);
    if (bResult == FALSE) {
		OutputDebugString("WSAResetEvent failed with error \n");
        //wprintf(L"WSAResetEvent failed with error = %d\n", WSAGetLastError());
    }
	if((Index - WSA_WAIT_EVENT_0) == 0){
		//read completed
		OnDataReceived();
	}
	if((Index - WSA_WAIT_EVENT_0) == 1){
		//read completed
		OnDataSent();
	}
}

//send new value for volume
void CCliNetwork::SendVolume(int vol)
{
	// send some data
    //-----------------------------------------
    // set the volume on the server to 50%
	sprintf_s(buffer, "%d\n", vol);
	DataBuf.len = strlen(buffer)+1;
    int iResult =
        WSASend(ConnectSocket, &DataBuf, 1, &RecvBytes, Flags, &WriteOverlapped, NULL);
    if (iResult != 0) {
		OutputDebugString("WSASend failed with error = \n");
        //wprintf(L"WSASend failed with error = %d\n", WSAGetLastError());
    }
}

//setup async read
void CCliNetwork::ReceiveVolume()
{
	DataBuf.len = DATA_BUFSIZE;

	if (WSARecv(ConnectSocket, &DataBuf, 1, &RecvBytes, &Flags, &ReadOverlapped, NULL) ==
		SOCKET_ERROR) {
		int iResult = WSAGetLastError();
		if (iResult != WSA_IO_PENDING){
			OutputDebugString("WSARecv failed with error \n");
			//wprintf(L"WSARecv failed with error = %d\n", iResult);
		}
	}
}

void CCliNetwork::OnDataReceived(){
	char outTextBuff[MAX_PATH];
	DWORD BytesTransferred = 0;

	//-----------------------------------------
    // Determine the status of the overlapped event
    BOOL bResult =
        WSAGetOverlappedResult(ConnectSocket, &ReadOverlapped, &BytesTransferred, FALSE,
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
        return ;
    }
	sprintf_s(outTextBuff, "The data received from server OK %d, %d\n", BytesTransferred, DataBuf.len);
	OutputDebugString(outTextBuff);
	ReceiveVolume();
}

void CCliNetwork::OnDataSent(){
	char outTextBuff[MAX_PATH];
	DWORD BytesTransferred = 0;

	//-----------------------------------------
    // Determine the status of the overlapped event
    BOOL bResult =
        WSAGetOverlappedResult(ConnectSocket, &WriteOverlapped, &BytesTransferred, FALSE,
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
        return ;
    }
	sprintf_s(outTextBuff, "The data sent to server OK %d, %d\n", BytesTransferred, DataBuf.len);
	OutputDebugString(outTextBuff);
}
