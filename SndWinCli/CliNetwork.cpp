#include "CliNetwork.h"
#include "resource.h"
#include "commctrl.h"
extern HWND g_hDlg;

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

int CCliNetwork::Connect(char *ipAddress, int port)
{
	int iResult;
	struct addrinfo *result = NULL;
	struct addrinfo *ptr = NULL;
	struct sockaddr_in  *sockaddr_ipv4 = NULL;

	struct addrinfo hints;
	char portStr[80];
	sprintf_s(portStr, "%d", port);

	//--------------------------------
	// Setup the hints address info structure
	// which is passed to the getaddrinfo() function
	ZeroMemory( &hints, sizeof(hints) );
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo(ipAddress, portStr, &hints, &result);
	if(iResult != 0){
		OutputDebugString("Failed to getaddrinfo\n");
		return 1;
	}

	for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {
		switch (ptr->ai_family) {
			case AF_INET:
				sockaddr_ipv4 = (struct sockaddr_in *) ptr->ai_addr;
				break;
		}
	}
	if(sockaddr_ipv4 == NULL){
		OutputDebugString("Failed to get ipv4 struct\n");
		return 1;
	}

    //----------------------
    // The sockaddr_in structure specifies the address family,
    // IP address, and port of the server to be connected to.
    sockaddr_in clientService;
	memcpy_s(&clientService, sizeof(sockaddr_in), sockaddr_ipv4, sizeof(sockaddr_in));
	freeaddrinfo(result);

    //----------------------
    // Connect to server.
    iResult = connect(ConnectSocket, (SOCKADDR *) & clientService, sizeof (clientService));
    if (iResult == SOCKET_ERROR) {
		char outTextBuff[MAX_PATH];
		sprintf_s(outTextBuff,"connect to %s failed with error: %ld\n", ipAddress, WSAGetLastError());
		OutputDebugString(outTextBuff);

		LPTSTR lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
					NULL,
					WSAGetLastError(),
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					(LPTSTR) &lpMsgBuf,
					0,
					NULL 
					);
		OutputDebugString((char*)lpMsgBuf);
		MessageBox(NULL,lpMsgBuf, outTextBuff, MB_ICONERROR);
		// Free the buffer.
		LocalFree( lpMsgBuf );

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
	//sprintf_s(buffer, "%d%s\n", vol, mute?"m":"u");
	//DataBuf.len = strlen(buffer)+1;
	buffer[0] = 4;			// size of package for client
	buffer[1] = RMT_VOLUME;	// function
	buffer[2] = vol;		// arg0
	buffer[3] = 0;			// arg1
	DataBuf.len = 4;		// size to send for sender
    int iResult =
        WSASend(ConnectSocket, &DataBuf, 1, &RecvBytes, Flags, &WriteOverlapped, NULL);
    if (iResult != 0) {
        //wprintf(L"WSASend failed with error = %d\n", WSAGetLastError());
		OutputDebugString("WSASend failed with error = \n");
    }
}

//send new value for mute
void CCliNetwork::SendMute(BOOL mute)
{
	// send some data
    //-----------------------------------------
    // set the volume on the server to 50%
	//sprintf_s(buffer, "%d%s\n", vol, mute?"m":"u");
	//DataBuf.len = strlen(buffer)+1;
	buffer[0] = 4;			// size of package for client
	buffer[1] = RMT_MUTE;	// function
	buffer[2] = mute;		// arg0
	buffer[3] = 0;			// arg1
	DataBuf.len = 4;		// size to send for sender
    int iResult =
        WSASend(ConnectSocket, &DataBuf, 1, &RecvBytes, Flags, &WriteOverlapped, NULL);
    if (iResult != 0) {
        //wprintf(L"WSASend failed with error = %d\n", WSAGetLastError());
		OutputDebugString("WSASend failed with error = \n");
    }
}

void CCliNetwork::SendLockCmd()
{
	buffer[0] = 4;			// size of package for client
	buffer[1] = RMT_LOCK;	// function
	buffer[2] = 0;			// arg0
	buffer[3] = 0;			// arg1
	DataBuf.len = 4;		// size to send for sender
    int iResult = WSASend(ConnectSocket, &DataBuf, 1, &RecvBytes, Flags, &WriteOverlapped, NULL);
    if (iResult != 0) {
        //wprintf(L"WSASend failed with error = %d\n", WSAGetLastError());
		OutputDebugString("WSASend failed with error = \n");
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
	UpdateDialog(BytesTransferred);
	ReceiveVolume();
}

void CCliNetwork::UpdateDialog(DWORD BytesTransferred){
    if (g_hDlg != NULL)
    {
		int volume[2] = {0,0};
		int mute[2] = {0,0};
		int lockCmd = 0;
		int pkgSize = 0;

		//process received buffer and store the actions
		char *recBytesPtr = DataBuf.buf;
		for(DWORD i = 0; i < BytesTransferred-1; i += pkgSize){
			pkgSize = recBytesPtr[0];
			if(pkgSize < 3) break;
			int function = recBytesPtr[1];
			switch(function){
			case RMT_VOLUME:
				volume[0] = 1;
				volume[1] = recBytesPtr[2];
				break;
			case RMT_MUTE:
				mute[0] = 1;
				mute[1] = recBytesPtr[2];
				break;
			case RMT_LOCK:
				lockCmd = 1;
				break;
			default:
				break;
			}
			recBytesPtr += pkgSize;
		}
		//execute the actions required
		if(volume[0] == 1)
			PostMessage(GetDlgItem(g_hDlg, IDC_SLIDER_VOLUME), TBM_SETPOS, TRUE, LPARAM(volume[1]));
		if(mute[0] == 1)
			PostMessage(GetDlgItem(g_hDlg, IDC_CHECK_MUTE), BM_SETCHECK, mute[1]==0?BST_UNCHECKED:BST_CHECKED, 0);
		//server does not send lock command
		//if(lockCmd == 1)
			//LockWorkStation();
    }
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
