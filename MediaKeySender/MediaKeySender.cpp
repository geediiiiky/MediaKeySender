// MediaKeySender.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "MediaKeySender.h"
#include "shellapi.h"
#include "assert.h"

#define WITH_MMKS 1

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

#if WITH_MMKS
void HandleMultiMediaKeyMapping();
void CheckKeyStatus();

const int TOTAL_KEYS = 300;
bool keyPressed[TOTAL_KEYS];
bool prevKeyPressed[TOTAL_KEYS];
#endif

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_MEDIAKEYSENDER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MEDIAKEYSENDER));

	MSG msg;

#if WITH_MMKS
	// Main message loop:
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		// check for messages
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
			//if (GetMessage(&msg, NULL, 0, 0))
		{
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			CheckKeyStatus();
			HandleMultiMediaKeyMapping();
		}

		Sleep(20);
	}
#else
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
#endif

    return (int) msg.wParam;
}


#if WITH_MMKS
enum MultiMediaKey
{
	MMK_Stop,
	MMK_ResumePause,
	MMK_Previous,
	MMK_Next,
	MMK_VolumeUp,
	MMK_VolumeDown,
	MMK_Mute,

	MMK_Count
};

bool isMultiMediaKeyTriggered[MMK_Count];
int MultiKeyMapping[MMK_Count] = {
	/*MMK_Stop,*/			VK_MEDIA_STOP,
	/*MMK_ResumePause,*/	VK_MEDIA_PLAY_PAUSE,
	/*MMK_Previous,*/		VK_MEDIA_PREV_TRACK,
	/*MMK_Next,*/			VK_MEDIA_NEXT_TRACK,
	/*MMK_VolumeUp,*/		VK_VOLUME_UP,
	/*MMK_VolumeDown,*/		VK_VOLUME_DOWN,
	/*MMK_Mute,*/			VK_VOLUME_MUTE
};

bool IsKeyDown(int key)
{
	SHORT retval = GetAsyncKeyState(key);
	if (HIBYTE(retval))
	{
		return true;
	}
	return false;
}

void SetKeyStatus(int key)
{
	keyPressed[key] = IsKeyDown(key);
}

void CheckKeyStatus()
{
	memcpy(prevKeyPressed, keyPressed, sizeof(bool) * TOTAL_KEYS);
	SetKeyStatus(VK_LCONTROL);
	SetKeyStatus(VK_RCONTROL);
	SetKeyStatus(VK_LMENU);
	SetKeyStatus(VK_RMENU);
	SetKeyStatus(VK_LSHIFT);
	SetKeyStatus(VK_RSHIFT);
	SetKeyStatus(VK_LEFT);
	SetKeyStatus(VK_RIGHT);
	SetKeyStatus(VK_UP);
	SetKeyStatus(VK_DOWN);
	SetKeyStatus('S');
	SetKeyStatus('P');
	SetKeyStatus('M');
}

bool IsKeyJustTapped(int key)
{
	return prevKeyPressed[key] && !keyPressed[key];
}

void HandleMultiMediaKeyMapping()
{
	bool isLeftControlDown = keyPressed[VK_LCONTROL];
	bool isRightControlDown = keyPressed[VK_RCONTROL];
	bool isControlDown = isLeftControlDown || isRightControlDown;

	bool isLeftAltDown = keyPressed[VK_LMENU];
	bool isRightAltDown = keyPressed[VK_RMENU];
	bool isAltDown = isLeftAltDown || isRightAltDown;

	bool isLeftShiftDown = keyPressed[VK_LSHIFT];
	bool isRightShiftDown = keyPressed[VK_RSHIFT];
	bool isShiftDown = isLeftShiftDown || isRightShiftDown;

	if (isControlDown && isAltDown && isShiftDown && IsKeyJustTapped('S'))
	{
		isMultiMediaKeyTriggered[MMK_Stop] = true;
	}
	else if (isControlDown && isAltDown && isShiftDown && IsKeyJustTapped('P'))
	{
		isMultiMediaKeyTriggered[MMK_ResumePause] = true;
	}
	else if (isControlDown && isAltDown && isShiftDown && IsKeyJustTapped(VK_LEFT))
	{
		isMultiMediaKeyTriggered[MMK_Previous] = true;
	}
	else if (isControlDown && isAltDown && isShiftDown && IsKeyJustTapped(VK_RIGHT))
	{
		isMultiMediaKeyTriggered[MMK_Next] = true;
	}
	else if (isControlDown && isAltDown && isShiftDown && IsKeyJustTapped(VK_UP))
	{
		isMultiMediaKeyTriggered[MMK_VolumeUp] = true;
	}
	else if (isControlDown && isAltDown && isShiftDown && IsKeyJustTapped(VK_DOWN))
	{
		isMultiMediaKeyTriggered[MMK_VolumeDown] = true;
	}
	else if (isControlDown && isAltDown && isShiftDown && IsKeyJustTapped('M'))
	{
		isMultiMediaKeyTriggered[MMK_Mute] = true;
	}
	//else
	{
		bool keyEventSent = false;
		for (int i = 0; i < MMK_Count; ++i)
		{
			if (isMultiMediaKeyTriggered[i])
			{
				assert(!keyEventSent);

				if (isLeftControlDown)
				{
					keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
				}
				if (isRightControlDown)
				{
					keybd_event(VK_RCONTROL, 0, KEYEVENTF_KEYUP, 0);
				}

				if (isLeftShiftDown)
				{
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				if (isRightShiftDown)
				{
					keybd_event(VK_RSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}

				if (isLeftAltDown)
				{
					keybd_event(VK_LMENU, 0, KEYEVENTF_KEYUP, 0);
				}
				if (isRightAltDown)
				{
					keybd_event(VK_RMENU, 0, KEYEVENTF_KEYUP, 0);
				}

				keybd_event(MultiKeyMapping[i], 0, 0, 0);
				keybd_event(MultiKeyMapping[i], 0, KEYEVENTF_KEYUP, 0);
				isMultiMediaKeyTriggered[i] = false;

				if (isLeftControlDown)
				{
					keybd_event(VK_LCONTROL, 0x1D, 0, 0);
				}
				if (isRightControlDown)
				{
					keybd_event(VK_RCONTROL, 0x1D, KEYEVENTF_EXTENDEDKEY, 0);
				}

				if (isLeftShiftDown)
				{
					keybd_event(VK_LSHIFT, 0x2A, 0, 0);
				}
				if (isRightShiftDown)
				{
					keybd_event(VK_RSHIFT, 0x36, 0, 0);
				}

				if (isLeftAltDown)
				{
					keybd_event(VK_LMENU, 0x38, 0, 0);
				}
				if (isRightAltDown)
				{
					keybd_event(VK_RMENU, 0x38, KEYEVENTF_EXTENDEDKEY, 0);
				}

				keyEventSent = true;
			}
		}
	}
}

#define WM_SHOWTASK WM_USER+1
NOTIFYICONDATA nid;
void MinimizeToTray(HWND hWnd)
{
	nid.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
	nid.hWnd = hWnd;
	nid.uID = IDR_MAINFRAME;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = WM_SHOWTASK;
	nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SMALL));
	wcscpy_s(nid.szTip, L"Test123");
	Shell_NotifyIcon(NIM_ADD, &nid);
	ShowWindow(hWnd, SW_HIDE);
}
#endif


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MEDIAKEYSENDER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MEDIAKEYSENDER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

#if WITH_MMKS
   MinimizeToTray(hWnd);
#else
   ShowWindow(hWnd, nCmdShow);
#endif

   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
#if WITH_MMKS
	case WM_SYSCOMMAND:
		if (wParam == SC_MINIMIZE)
		{
			MinimizeToTray(hWnd);
		}
		else
		{
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_SHOWTASK:
		if (lParam == WM_LBUTTONDBLCLK)
		{
			ShowWindow(hWnd, SW_SHOW);
		}
		break;
#endif
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
