#pragma once
class CSrvApp
{
public:
	CSrvApp(void);
	~CSrvApp(void);
	int ProcessClient(char* buffer, unsigned long &len, int *cliResult);

};

enum _func_enum {
	RMT_LOGIN,
	RMT_VOLUME,
	RMT_MUTE,
	RMT_LOCK,
	RMT_CTRLP,
	RMT_SLEEP,
	RMT_LOGOUT
};
