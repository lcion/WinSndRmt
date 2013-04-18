//application entry point
//command line pharsing
#define WIN32_LEAN_AND_MEAN

#include <stdio.h>

#include "SrvApp.h"
#include "SrvNetwrk.h"
#include "SrvAudio.h"

int PharseCmdLineArgs(char *lpCmdLine, char **ipAddress){
	int argn = 0;
	char *next_token;
	char *token = strtok_s(lpCmdLine, " ", &next_token);
	while(token){
		//use the token
		if(argn == 0) *ipAddress = token;
		else if(argn == 1) break;
		argn++;
		token = strtok_s(NULL, " ", &next_token);
	}
	return argn;
}


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
	char *ipAddress = "";
	PharseCmdLineArgs(lpCmdLine, &ipAddress);

	hr = volAudio.Initialize(&srvNetork);
	if(FAILED(hr)) return -1;

	hr = srvNetork.Initialize(ipAddress);
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

	    //-----------------------------------------
    // Create an event handle for the audio events
    EventArray[EventTotal] = WSACreateEvent();
    if (EventArray[EventTotal] == WSA_INVALID_EVENT) {
        OutputDebugString("WSACreateEvent failed with error\n");
        return 1;
    }
	srvNetork.SetAudioEvent(EventArray[EventTotal]);
    EventTotal++;

	//accept loop
	while (1) {
	  hr = srvNetork.Accept();
	  if(FAILED(hr)) return -1;

	  volAudio.PostVolValToClient();
	  srvNetork.ReceiveAsync();

      //-----------------------------------------
      // client loop Process overlapped receives on the socket
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
				//break out of the client loop and go find another client
				break;
		}
		else if((Index - WSA_WAIT_EVENT_0) == 1)
		{
			//write operation completed
			srvNetork.OnDataSent();
		}
		else if((Index - WSA_WAIT_EVENT_0) == 2)
		{
			//write operation completed
			srvNetork.OnSndEvent();
		}
	  }
	  // client disconnected close socket
	  srvNetork.CloseClientSocket();
	}

    return 0;
}
