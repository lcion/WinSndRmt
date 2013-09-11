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
			cliResult[4] = 1;
			break;
		default:
			break;
		}
		recBytesPtr += pkgSize;
	}
	return 0;
}
