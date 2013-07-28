#include "IpAddrDlg.h"
#include "resource.h"
#include <stdio.h>
#include <stdlib.h>

#define IPSTRLEN 80
INT_PTR CALLBACK OnIPAddressMsg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
TCHAR comboResult[IPSTRLEN];
extern HINSTANCE g_hInst;

CIpAddrDlg::CIpAddrDlg(void)
{
}


CIpAddrDlg::~CIpAddrDlg(void)
{
}

int CIpAddrDlg::GetIpAddrStrFromUser(HINSTANCE hInst, char **ipAddress)
{
	DialogBox(hInst, MAKEINTRESOURCE(IDD_IP_ADDR_DLG), NULL, OnIPAddressMsg);
	TCHAR *ptr = strchr(comboResult, ',');
	if(ptr)
		*ipAddress = ptr+1;
	else
		*ipAddress = comboResult;
	if(strlen(*ipAddress) > 0)
		return 1;
	return 0;
}

#include <list>
#include <fstream>
#include <Shlobj.h>
#include <Shlwapi.h>

using namespace std;

void ReadIPsFromFile(list<string> &ipList){
	ifstream src;
	TCHAR szPath[MAX_PATH];

	if(FAILED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA|CSIDL_FLAG_CREATE, 
                             NULL, 0, szPath))) return;

	PathAppend(szPath, TEXT("LucianIon"));
	if(!PathFileExists(szPath)) CreateDirectory(szPath, NULL);
	PathAppend(szPath, TEXT("WinSndRmt"));
	if(!PathFileExists(szPath)) CreateDirectory(szPath, NULL);
	PathAppend(szPath, TEXT("addrList.txt"));
	if(!PathFileExists(szPath)) return;

	src.open(szPath);
	if(src.is_open() == FALSE)
		return;
	char linestr[60];
	do{
		src.getline(linestr,60);
		if(strlen(linestr)>0)
			ipList.push_back(linestr);
	}while(!src.eof());

	src.close();
}

void WriteIPsToFile(char *linestr){
	ofstream dest;
	list<string> ipList;
	list<string>::iterator it;
	ReadIPsFromFile(ipList);

	//check if the string is already on top of the list
	it = ipList.begin();
	if(it != ipList.end())
		if(strcmp(linestr, (*it).c_str())==0)
			return;
	//check if the string is down in the list remove it
	for(it = ipList.begin(); it != ipList.end(); it++ ){
		if(strcmp(linestr, (*it).c_str())==0)
			break;
	}
	if(it != ipList.end())
		ipList.erase(it);

	TCHAR szPath[MAX_PATH];
	if(FAILED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA|CSIDL_FLAG_CREATE, 
                             NULL, 0, szPath))) return;

	PathAppend(szPath, TEXT("LucianIon"));
	if(!PathFileExists(szPath)) CreateDirectory(szPath, NULL);
	PathAppend(szPath, TEXT("WinSndRmt"));
	if(!PathFileExists(szPath)) CreateDirectory(szPath, NULL);
	PathAppend(szPath, TEXT("addrList.txt"));

	dest.open(szPath);
	if(dest.is_open() == FALSE)
		return;

	//write the new connection on top
	dest << linestr << endl;
	//write all other history
	for(it = ipList.begin(); it != ipList.end(); it++ )
		dest << (*it).c_str() << endl;

	dest.close();
}

// Message handler for IP Address dialog.
INT_PTR CALLBACK OnIPAddressMsg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		{//setup dialog icon
			HICON hIconSmall =(HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_SNDWINCLI), IMAGE_ICON,16, 16, 0);
			HICON hIconLarge =(HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_SNDWINCLI), IMAGE_ICON,256, 256, 0); // Big for task bar, small loaded otherwise.

			SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall) ;
			SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIconLarge) ;
		}

		//load list from file and populate combo box
		{
			list<string> ipList;
			ReadIPsFromFile(ipList);
			HWND hWndCombo = GetDlgItem(  hDlg, IDC_IPADDR_COMBO);
			for(list<string>::iterator it = ipList.begin(); it != ipList.end(); it++ )
				SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM)(*it).c_str());

			// Send the CB_SETCURSEL message to display an initial item 
			//  in the selection field
 			SendMessage(hWndCombo, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
		}

		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			//save ip to file
			if (LOWORD(wParam) == IDOK)
			{
				HWND hWndCombo = GetDlgItem(  hDlg, IDC_IPADDR_COMBO);
				GetWindowText(hWndCombo, comboResult, IPSTRLEN);
				OutputDebugString(comboResult);
				WriteIPsToFile(comboResult);
			}else{
			strcpy_s(comboResult, TEXT(""));
			}
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
