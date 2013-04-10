#define WIN32_LEAN_AND_MEAN

#include "CliNetwork.h"
#include <commctrl.h>
#include "Resource.h"
#include "IpAddrDlg.h"

// Need to link with Ws2_32.lib
#pragma comment(lib, "ws2_32.lib")

#define MAX_VOL 100

BOOL CALLBACK VolDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

int PharseCmdLineArgs(HINSTANCE hInst, char *lpCmdLine, char **ipAddress){
	int argn = 0;
	char *next_token;
	char *token = strtok_s(lpCmdLine, " ", &next_token);
	while(token){
		//use the token
		if(argn == 0) *ipAddress = token;
		else if(argn == 1) break;
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
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    HRESULT hr = S_OK;
	DWORD Index = 0;
	BOOL bResult = TRUE;
	char *ipAddress;
	CCliNetwork myNetwork;
	gCliNetwork = &myNetwork;

	//pharse the command line parameters expecting arguments ex: 192.168.1.12 10
	if(PharseCmdLineArgs(hInstance, lpCmdLine, &ipAddress) < 1)return 1;

	if(myNetwork.Initialize()) return 1;

	if(myNetwork.Connect(ipAddress)) return 1;
    //wprintf(L"Connected to server.\n");
	OutputDebugString("Connected to server.\n");

	InitCommonControls();
    DialogBox(hInstance, "VOLUMECONTROL", NULL, (DLGPROC)VolDlgProc);

	return 0;
}

//-----------------------------------------------------------
// VolDlgProc -- Dialog box procedure
//-----------------------------------------------------------
HWND g_hDlg = NULL;

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
        SendDlgItemMessage(hDlg, IDC_SLIDER_VOLUME, TBM_SETRANGEMIN, FALSE, 0);
        SendDlgItemMessage(hDlg, IDC_SLIDER_VOLUME, TBM_SETRANGEMAX, FALSE, MAX_VOL);
        //hr = g_pEndptVol->GetMute(&bMute);
        //ERROR_CANCEL(hr)
        SendDlgItemMessage(hDlg, IDC_CHECK_MUTE, BM_SETCHECK,
                           bMute ? BST_CHECKED : BST_UNCHECKED, 0);
        //hr = g_pEndptVol->GetMasterVolumeLevelScalar(&fVolume);
        //ERROR_CANCEL(hr)
        nVolume = (int)(MAX_VOL*fVolume + 0.5);
        SendDlgItemMessage(hDlg, IDC_SLIDER_VOLUME, TBM_SETPOS, TRUE, nVolume);

		// setup timer to process network events
		SetTimer(hDlg, 0, 100, NULL);
        return TRUE;

    case WM_TIMER:
		//OutputDebugString("On Timer\n");
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
            //hr = g_pEndptVol->SetMute(FALSE, &g_guidMyContext);
            //ERROR_CANCEL(hr)
            nVolume = SendDlgItemMessage(hDlg, IDC_SLIDER_VOLUME, TBM_GETPOS, 0, 0);
			OutputDebugString("Send new Volume to server.\n");
			gCliNetwork->SendVolume(nVolume);
            //fVolume = (float)nVolume/MAX_VOL;
            //hr = g_pEndptVol->SetMasterVolumeLevelScalar(fVolume, &g_guidMyContext);
            //ERROR_CANCEL(hr)
            return TRUE;
        }
        break;

    case WM_COMMAND:
        switch ((int)LOWORD(wParam))
        {
        case IDC_CHECK_MUTE:
            // The user selected the Mute check box in the dialog box.
            nChecked = SendDlgItemMessage(hDlg, IDC_CHECK_MUTE, BM_GETCHECK, 0, 0);
            bMute = (BST_CHECKED == nChecked);
            //hr = g_pEndptVol->SetMute(bMute, &g_guidMyContext);
            //ERROR_CANCEL(hr)
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
