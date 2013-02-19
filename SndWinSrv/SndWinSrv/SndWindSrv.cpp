#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>

#include "SrvApp.h"
#include "Epvolume.h"

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#define DATA_BUFSIZE 4096
static IAudioEndpointVolume *g_pEndptVol = NULL;
GUID g_guidMyContext = GUID_NULL;

#define EXIT_ON_ERROR(hr)  \
              if (FAILED(hr)) { goto Exit; }

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine,
                     int nCmdShow)
{
    HRESULT hr = S_OK;
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
    WSAOVERLAPPED AcceptOverlapped;
    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET AcceptSocket = INVALID_SOCKET;

    DWORD Index;
	CSrvApp myAppLogic;

	//snd specific initialization
	    IMMDeviceEnumerator *pEnumerator = NULL;
    IMMDevice *pDevice = NULL;
    CAudioEndpointVolumeCallback EPVolEvents;

    if (hPrevInstance)
    {
        return 0;
    }

    CoInitialize(NULL);

    hr = CoCreateGuid(&g_guidMyContext);
    EXIT_ON_ERROR(hr)

    // Get enumerator for audio endpoint devices.
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),
                          NULL, CLSCTX_INPROC_SERVER,
                          __uuidof(IMMDeviceEnumerator),
                          (void**)&pEnumerator);
    EXIT_ON_ERROR(hr)

	// Get default audio-rendering device.
    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    EXIT_ON_ERROR(hr)

	hr = pDevice->Activate(__uuidof(IAudioEndpointVolume),
                           CLSCTX_ALL, NULL, (void**)&g_pEndptVol);
    EXIT_ON_ERROR(hr)

	hr = g_pEndptVol->RegisterControlChangeNotify(
                     (IAudioEndpointVolumeCallback*)&EPVolEvents);
    EXIT_ON_ERROR(hr)

    //-----------------------------------------
    // Initialize Winsock
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
    // Create an event handle and setup an overlapped structure.
    EventArray[EventTotal] = WSACreateEvent();
    if (EventArray[EventTotal] == WSA_INVALID_EVENT) {
        OutputDebugString("WSACreateEvent failed with error\n");
		//wprintf(L"WSACreateEvent failed with error = %d\n", WSAGetLastError());
        closesocket(AcceptSocket);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    ZeroMemory(&AcceptOverlapped, sizeof (WSAOVERLAPPED));
    AcceptOverlapped.hEvent = EventArray[EventTotal];

    DataBuf.len = DATA_BUFSIZE;
    DataBuf.buf = buffer;

    EventTotal++;

    //-----------------------------------------
    // Call WSARecv to receive data into DataBuf on 
    // the accepted socket in overlapped I/O mode
    if (WSARecv(AcceptSocket, &DataBuf, 1, &RecvBytes, &Flags, &AcceptOverlapped, NULL) ==
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
        //-----------------------------------------
        // Determine the status of the overlapped event
        bResult =
            WSAGetOverlappedResult(AcceptSocket, &AcceptOverlapped, &BytesTransferred, FALSE,
                                   &Flags);
        if (bResult == FALSE) {
			OutputDebugString("WSAGetOverlappedResult failed with error ");
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
			float fVolume = (float)vol/MAX_VOL;
            hr = g_pEndptVol->SetMasterVolumeLevelScalar(fVolume, &g_guidMyContext);
		}
        //-----------------------------------------
        // If data has been received, echo the received data
        // from DataBuf back to the client
        iResult =
            WSASend(AcceptSocket, &DataBuf, 1, &RecvBytes, Flags, &AcceptOverlapped, NULL);
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
		//-----------------------------------------
		// Call WSARecv to receive data into DataBuf on 
		// the accepted socket in overlapped I/O mode
		if (WSARecv(AcceptSocket, &DataBuf, 1, &RecvBytes, &Flags, &AcceptOverlapped, NULL) ==
			SOCKET_ERROR) {
			iResult = WSAGetLastError();
			if (iResult != WSA_IO_PENDING){
				OutputDebugString("WSARecv failed with error \n");
				//wprintf(L"WSARecv failed with error = %d\n", iResult);
			}
		}

    }

Exit:
    if (FAILED(hr))
    {
        MessageBox(NULL, TEXT("This program requires Windows Vista."),
                   TEXT("Error termination"), MB_OK);
    }
    if (pEnumerator != NULL)
    {
        g_pEndptVol->UnregisterControlChangeNotify(
                    (IAudioEndpointVolumeCallback*)&EPVolEvents);
    }
    SAFE_RELEASE(pEnumerator)
    SAFE_RELEASE(pDevice)
    SAFE_RELEASE(g_pEndptVol)
    CoUninitialize();

    closesocket(ListenSocket);
    closesocket(AcceptSocket);
    WSACleanup();

    return 0;
}
