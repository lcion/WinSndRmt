//application entry point
//command line pharsing
#define WIN32_LEAN_AND_MEAN

#include <stdio.h>

#include "SrvApp.h"
#include "SrvNetwrk.h"
#include "SrvAudio.h"

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine,
                     int nCmdShow)
{
	//-----------------------------------------
    // Declare and initialize variables
    int iResult = 0;
    BOOL bResult = TRUE;
    HRESULT hr = S_OK;

	CSrvAudio volAudio;
	CSrvApp myAppLogic;
	CSrvNetwrk srvNetork(volAudio, myAppLogic);

    WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
	DWORD EventTotal = 0;

    DWORD Index;

    if (hPrevInstance)
    {
        return 0;
    }

	hr = volAudio.Initialize();
	if(FAILED(hr)) return -1;

	hr = srvNetork.Initialize();
	if(FAILED(hr)) return -1;

	hr = srvNetork.Accept();
	if(FAILED(hr)) return -1;

	//-----------------------------------------
    // Create an event handle and setup an overlapped structure for read.
    EventArray[EventTotal] = WSACreateEvent();
    if (EventArray[EventTotal] == WSA_INVALID_EVENT) {
        OutputDebugString("WSACreateEvent failed with error\n");
        return 1;
    }

	srvNetork.SetReadEvent(EventArray[EventTotal]);

    EventTotal++;

    //-----------------------------------------
    // Create an event handle and setup an overlapped structure for write.
    EventArray[EventTotal] = WSACreateEvent();
    if (EventArray[EventTotal] == WSA_INVALID_EVENT) {
        OutputDebugString("WSACreateEvent failed with error\n");
        return 1;
    }

    //WriteOverlapped.hEvent = EventArray[EventTotal];
	srvNetork.SetWriteEvent(EventArray[EventTotal]);

    EventTotal++;

	srvNetork.ReceiveAsync();

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
        }

		if((Index - WSA_WAIT_EVENT_0) == 0)
		{
			//read operation completed
			if( srvNetork.OnDataReceived() < 0 )
				return -1;

		}else
		{
			//write operation completed
			srvNetork.OnDataSent();
		}
    }

    return 0;
}
