#include "IpAddrDlg.h"
#include "resource.h"


INT_PTR CALLBACK OnIPAddressMsg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

CIpAddrDlg::CIpAddrDlg(void)
{
}


CIpAddrDlg::~CIpAddrDlg(void)
{
}

int CIpAddrDlg::GetIpAddrStrFromUser(HINSTANCE hInst, char **ipAddress)
{
	DialogBox(hInst, MAKEINTRESOURCE(IDD_IP_ADDR_DLG), NULL, OnIPAddressMsg);
	return 0;
}

// Message handler for IP Address dialog.
INT_PTR CALLBACK OnIPAddressMsg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		//load list from file and populate combo box
		{
			TCHAR comboTxt[] = TEXT("192.168.0.1");
			TCHAR comboTxt1[] = TEXT("192.168.0.2");
			HWND hWndCombo = GetDlgItem(  hDlg, IDC_IPADDR_COMBO);
			SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM)comboTxt);
			SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM)comboTxt1);

			// Send the CB_SETCURSEL message to display an initial item 
			//  in the selection field
 			SendMessage(hWndCombo, CB_SETCURSEL, (WPARAM)1, (LPARAM)0);
		}

		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			//save ip to file
			if (LOWORD(wParam) == IDOK)
			{
				HWND hWndCombo = GetDlgItem(  hDlg, IDC_IPADDR_COMBO);
				TCHAR comboResult[60];
				GetWindowText(hWndCombo, comboResult, 60);
				OutputDebugString(comboResult);
			}
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
