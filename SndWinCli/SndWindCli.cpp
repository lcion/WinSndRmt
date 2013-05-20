#define WIN32_LEAN_AND_MEAN

#include "CliNetwork.h"
#include <commctrl.h>
#include <stdlib.h>
#include "Resource.h"
#include "IpAddrDlg.h"

#define MAX_VOL 100

BOOL CALLBACK VolDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

int PharseCmdLineArgs(HINSTANCE hInst, char *lpCmdLine, char **ipAddress, int *port){
	int argn = 0;
	char *next_token;
	char *token = strtok_s(lpCmdLine, " ", &next_token);
	while(token){
		//use the token
		if(argn == 0) *ipAddress = token;
		else if(argn == 1){
			if(isdigit(token[0])){
				int portNo = atoi(token);
				if(portNo > 1000)
					*port = portNo;
			}
		}
		else if(argn == 2) break;
		argn++;
		token = strtok_s(NULL, " ", &next_token);
	}
	//get ip address from the user
	if(argn < 1)
	{
		CIpAddrDlg myIpAddrDlg;
		argn = myIpAddrDlg.GetIpAddrStrFromUser(hInst, ipAddress);
	}
	return argn;
}

CCliNetwork *gCliNetwork;
char *gIpAddress;
HWND g_hDlg = NULL;
HINSTANCE g_hInst;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    HRESULT hr = S_OK;
	DWORD Index = 0;
	BOOL bResult = TRUE;
	g_hInst = hInstance;
	CCliNetwork myNetwork;
	gCliNetwork = &myNetwork;
	int port = 27015;

	InitCommonControls();
	//pharse the command line parameters expecting arguments ex: 192.168.0.12 10234
	if(PharseCmdLineArgs(hInstance, lpCmdLine, &gIpAddress, &port) < 1)return 1;

	if(myNetwork.Initialize()) return 1;

	if(myNetwork.Connect(gIpAddress, port)) return 1;
	OutputDebugString("Connected to server.\n");

    DialogBox(hInstance, MAKEINTRESOURCE(IDD_VOL_CONTROL_DLG), NULL, (DLGPROC)VolDlgProc);

	return 0;
}

//-----------------------------------------------------------
// VolDlgProc -- Dialog box procedure
//-----------------------------------------------------------
BOOL CALLBACK VolDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    BOOL bMute = FALSE;
    float fVolume = 0.0f;
    int nVolume = 0;
    int nChecked;

    switch (message)
    {
    case WM_INITDIALOG:
        g_hDlg = hDlg;
		{//setup dialog icon
			HICON hIconSmall =(HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_SNDWINCLI), IMAGE_ICON,16, 16, 0);
			HICON hIconLarge =(HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_SNDWINCLI), IMAGE_ICON,256, 256, 0); // Big for task bar, small loaded otherwise.

			SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall) ;
			SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIconLarge) ;
		}

		{ //set the title of the main window dialog
			char dialogTitle[100];
			sprintf_s(dialogTitle, "%s %s", "Audio Remote Volume", gIpAddress);
			SetWindowText(hDlg, dialogTitle);
		}
        SendDlgItemMessage(hDlg, IDC_SLIDER_VOLUME, TBM_SETRANGEMIN, FALSE, 0);
        SendDlgItemMessage(hDlg, IDC_SLIDER_VOLUME, TBM_SETRANGEMAX, FALSE, MAX_VOL);
        SendDlgItemMessage(hDlg, IDC_CHECK_MUTE, BM_SETCHECK,
                           bMute ? BST_CHECKED : BST_UNCHECKED, 0);
        nVolume = (int)(MAX_VOL*fVolume + 0.5);
        SendDlgItemMessage(hDlg, IDC_SLIDER_VOLUME, TBM_SETPOS, TRUE, nVolume);

		// setup timer to process network events
		SetTimer(hDlg, 0, 100, NULL);
        return TRUE;

    case WM_TIMER:
		gCliNetwork->DoTimerProc();
		break;
    case WM_HSCROLL:
        switch (LOWORD(wParam))
        {
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
        case SB_LINERIGHT:
        case SB_LINELEFT:
        case SB_PAGERIGHT:
        case SB_PAGELEFT:
        case SB_RIGHT:
        case SB_LEFT:
            // The user moved the volume slider in the dialog box.
            SendDlgItemMessage(hDlg, IDC_CHECK_MUTE, BM_SETCHECK, BST_UNCHECKED, 0);
            nVolume = SendDlgItemMessage(hDlg, IDC_SLIDER_VOLUME, TBM_GETPOS, 0, 0);
			OutputDebugString("Send new Volume to server.\n");
			gCliNetwork->SendVolume(nVolume, FALSE);
            return TRUE;
        }
        break;

    case WM_COMMAND:
        switch ((int)LOWORD(wParam))
        {
        case IDC_CHECK_MUTE:
            // The user selected the Mute check box in the dialog box.
            nVolume = SendDlgItemMessage(hDlg, IDC_SLIDER_VOLUME, TBM_GETPOS, 0, 0);
            nChecked = SendDlgItemMessage(hDlg, IDC_CHECK_MUTE, BM_GETCHECK, 0, 0);
            bMute = (BST_CHECKED == nChecked);
			gCliNetwork->SendVolume(nVolume, bMute);

            return TRUE;
        case IDCANCEL:
			KillTimer(hDlg, 0);
            EndDialog(hDlg, TRUE);
            return TRUE;
        }
        break;
    }
    return FALSE;
}
