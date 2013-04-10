#pragma once

#include <windows.h>
class CIpAddrDlg
{
public:
	CIpAddrDlg(void);
	~CIpAddrDlg(void);

	int GetIpAddrStrFromUser(HINSTANCE hInst, char **ipAddress);
};

