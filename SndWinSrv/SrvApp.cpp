//used to handle application logic
#include "SrvApp.h"
#include <stdio.h>
#include <string>


CSrvApp::CSrvApp(void)
{
}


CSrvApp::~CSrvApp(void)
{
}


int CSrvApp::ProcessClient(char* buffer, unsigned long &len, int *cliResult)
{
	int pkgSize = 0;
	//process received buffer and store the actions
	char *recBytesPtr = buffer;
	for(unsigned long i = 0; i < len-1; i += pkgSize){
		pkgSize = recBytesPtr[0];
		if(pkgSize < 3) break;
		int function = recBytesPtr[1];
		switch(function){
		case RMT_VOLUME:
			cliResult[0] = 1;
			cliResult[1] = recBytesPtr[2];
			break;
		case RMT_MUTE:
			cliResult[2] = 1;
			cliResult[3] = recBytesPtr[2];
			break;
		case RMT_LOCK:
			LockWorkStation();
			break;
		case RMT_CTRLP:
			OnCtrlP();
			break;
		case RMT_SLEEP:
			break;
		case RMT_MOUSE_DOWN_UP:
			OnMouseDownUp(recBytesPtr[2]);
			break;
		case RMT_MOUSE_MOVE:
			OnMouseMove(recBytesPtr[2], recBytesPtr[3]);
			break;

		default:
			break;
		}
		recBytesPtr += pkgSize;
	}
	return 0;
}

void CSrvApp::OnMouseDownUp(char mseBtn){
	INPUT mseEvent = { 0 };
	mseEvent.type = INPUT_MOUSE;
	mseEvent.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	SendInput(1, &mseEvent, sizeof(mseEvent));
	mseEvent.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	SendInput(1, &mseEvent, sizeof(mseEvent));
	OutputDebugString("On Mouse Down Up\n");
}

void CSrvApp::OnMouseMove(char dx, char dy){
	INPUT mseEvent = { 0 };
	mseEvent.type = INPUT_MOUSE;
	mseEvent.mi.dwFlags = MOUSEEVENTF_MOVE;
	mseEvent.mi.dx = dx;
	mseEvent.mi.dy = dy;
	SendInput(1, &mseEvent, sizeof(mseEvent));
}

void CSrvApp::OnCtrlP(){
	INPUT keyEvent = { 0 };
	keyEvent.ki.wVk = VK_CONTROL; // control
	keyEvent.ki.wScan = 0;
	keyEvent.type = INPUT_KEYBOARD;
	SendInput(1, &keyEvent, sizeof(keyEvent));

	//send 'P' key
	keyEvent.ki.wVk = 80; // 'P'=80
	SendInput(1, &keyEvent, sizeof(keyEvent));

	// Release the 'P' key
	keyEvent.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
	SendInput(1, &keyEvent, sizeof(INPUT));

	// Release the "ctrl" key
	keyEvent.ki.wVk = VK_CONTROL; // ctrl up
	SendInput(1, &keyEvent, sizeof(INPUT));
}
