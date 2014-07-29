#pragma once
#include <windows.h>

class CSrvApp
{
public:
	CSrvApp(void);
	~CSrvApp(void);
	int ProcessClient(char* buffer, unsigned long &len, int *cliResult);
	void OnCtrlP();
	void OnMouseDownUp(char mseBtn);
	void OnMouseMove(char dx, char dy);
};

enum _func_enum {
	RMT_LOGIN,
	RMT_VOLUME,
	RMT_MUTE,
	RMT_LOCK,
	RMT_CTRLP,
	RMT_SLEEP,
	RMT_MOUSE_DOWN_UP,
	RMT_MOUSE_MOVE,
	RMT_LOGOUT
};
