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


int CSrvApp::ProcessClient(char* buffer, unsigned long &len, int &bMute)
{
	if(len > 5){
		sprintf_s(buffer, 100, "too long\n");
		len = strlen(buffer) + 1;
		return -1;
	}
	int srLen = strlen(buffer);
	if(srLen > 5){
		sprintf_s(buffer, 100, "bad format1\n");
		len = strlen(buffer) + 1;
		return -1;
	}
	if(!isdigit(buffer[0])){
		sprintf_s(buffer, 100, "bad format2\n");
		len = strlen(buffer) + 1;
		return -1;
	}
	int vol = atoi(buffer);
	if(vol < 0 || vol > 100){
		sprintf_s(buffer, 100, "OutOfRange\n");
		len = strlen(buffer) + 1;
		return -1;
	}
	//pik up the mute if is set
	if(buffer[srLen-2] == 'u'){
		bMute = 0;
	}else if(buffer[srLen-2] == 'm'){
		bMute = 1;
	}


	return vol;
}
