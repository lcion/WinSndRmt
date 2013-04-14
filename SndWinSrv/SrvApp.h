#pragma once
class CSrvApp
{
public:
	CSrvApp(void);
	~CSrvApp(void);
	int ProcessClient(char* buffer, unsigned long &len, int &bMute);
};

