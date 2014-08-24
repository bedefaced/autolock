#include "stdafx.h"
#include "autolock.h"

// global variables
HINSTANCE hInst;
HWND hWnd;
HPOWERNOTIFY hn;
BOOL suspended;

// forward declarations of functions included in this code module
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	suspended = false;

	MSG msg;

	MyRegisterClass (hInstance);

	// perform application initialization
	if (!InitInstance (hInstance, nCmdShow))
		return FALSE;

    hn = RegisterPowerSettingNotification (hWnd, &GUID_LIDSWITCH_STATE_CHANGE, 0);
	if (!hn) 
	{
	   MessageBox (0, TEXT("Cannot initialise notification. Please restart manually."), TEXT("Error"), 0);
	   return FALSE;
	}

	// main message loop
	while (GetMessage (&msg, NULL, 0, 0))
	{
		TranslateMessage (&msg);
		DispatchMessage (&msg);
	}

	UnregisterPowerSettingNotification (hn);

	return (int) msg.wParam;
}



ATOM MyRegisterClass (HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize		   = sizeof (WNDCLASSEX);
	wcex.style		   = 0;
	wcex.lpfnWndProc   = WndProc;
	wcex.cbClsExtra	   = 0;
	wcex.cbWndExtra	   = 0;
	wcex.hInstance	   = hInstance;
	wcex.hIcon		   = NULL;
	wcex.hCursor	   = NULL;
	wcex.hbrBackground = 0;
	wcex.lpszMenuName  = NULL;
	wcex.lpszClassName = TEXT("main");
	wcex.hIconSm	   = NULL;

	return RegisterClassEx (&wcex);
}

BOOL InitInstance (HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // store instance handle in our global variable

   hWnd = CreateWindow (TEXT("main"), TEXT(""), WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
      return FALSE;

   return TRUE;
}

LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	POWERBROADCAST_SETTING *setting;

	switch (message)
	{
		case WM_DESTROY:
			PostQuitMessage (0);
			break;

		case WM_POWERBROADCAST:
			if (wParam == PBT_POWERSETTINGCHANGE)
			{
				setting = (POWERBROADCAST_SETTING *) lParam;
				if (setting->PowerSetting == GUID_LIDSWITCH_STATE_CHANGE)
				{
					if ( ((int)(*setting->Data) == 0) && (!suspended) )
						LockWorkStation (); // lid is closed
				}
			}
			if (wParam == PBT_APMSUSPEND || wParam == PBT_APMQUERYSUSPEND) suspended = true;
			if (wParam == PBT_APMRESUMEAUTOMATIC) suspended = false;
			break;

		default:
			return DefWindowProc (hWnd, message, wParam, lParam);
	}

	return 0;
}
