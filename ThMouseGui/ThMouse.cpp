#include "framework.h"
#include "resource.h"

import common.datatype;
import main.readconfig;
import core.windowshook;

constexpr auto MAX_LOADSTRING = 100;

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
TCHAR szBalloonInfo[MAX_LOADSTRING];
UINT WM_TASKBARCREATED;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
bool				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    auto hMutex = CreateMutex(NULL, TRUE, "ThMouse");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBox(NULL, "ThMouseX is already running.", "ThMouseX", MB_OK | MB_ICONINFORMATION);
        return 1;
    }

    auto *pConfig = new GameConfigArray{};
    if (!readGamesFile(pConfig))
        return 1;

    int leftButton = 0;
    int midButton = 0;
    char *pTextureFilePath = new char[512];
    if (!readIniFile(&leftButton, &midButton, pTextureFilePath))
        return 1;

    char *pTextureFileFullPath = new char[512];
    GetFullPathName(pTextureFilePath, 512, pTextureFileFullPath, NULL);
    delete[] pTextureFilePath;

    if (!installThDxHook(pConfig, leftButton, midButton, pTextureFileFullPath))
        return 0;

    delete[] pTextureFileFullPath;
    delete pConfig;

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_THMOUSE, szWindowClass, MAX_LOADSTRING);
    LoadString(hInstance, IDS_BALLOON_INFO, szBalloonInfo, MAX_LOADSTRING);

    WM_TASKBARCREATED = RegisterWindowMessage(TEXT("TaskbarCreated"));

    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow)) {
        return 0;
    }

    MSG msg;
    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    removeThDxHook();

    return (int)msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance) {
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_THMOUSE));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
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
bool InitInstance(HINSTANCE hInstance, int nCmdShow) {
    HWND hWnd;

    hInst = hInstance; // Store instance handle in our global variable

    //hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
    //   CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
    hWnd = CreateWindowEx(WS_EX_TOOLWINDOW, szWindowClass, szTitle, WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

    if (!hWnd) {
        return false;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return true;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    NOTIFYICONDATA nid;

    switch (message) {
        //case WM_COMMAND:
        //	wmId    = LOWORD(wParam);
        //	wmEvent = HIWORD(wParam);
        //	// Parse the menu selections:
        //	switch (wmId)
        //	{
        //	case IDM_ABOUT:
        //		DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
        //		break;
        //	case IDM_EXIT:
        //		DestroyWindow(hWnd);
        //		break;
        //	default:
        //		return DefWindowProc(hWnd, message, wParam, lParam);
        //	}
        //	break;
        case WM_CREATE:
            ZeroMemory(&nid, sizeof(nid));
            nid.cbSize = sizeof(nid);
            nid.hWnd = hWnd;
            nid.uID = 0;
            nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_INFO | NIF_TIP;
            nid.uCallbackMessage = WM_USER;
            nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_THMOUSE));
            lstrcpy(nid.szTip, szTitle);
            lstrcpy(nid.szInfoTitle, szTitle);
            lstrcpy(nid.szInfo, szBalloonInfo);
            Shell_NotifyIcon(NIM_ADD, &nid);
            break;
        case WM_USER:
            switch (lParam) {
                case WM_LBUTTONDBLCLK:
                    if (DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About) == IDC_ExitButton)
                        SendMessage(hWnd, WM_CLOSE, wParam, lParam);
                    break;

                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        case WM_DESTROY:
            ZeroMemory(&nid, sizeof(nid));
            nid.cbSize = sizeof(nid);
            nid.hWnd = hWnd;
            nid.uID = 0;
            Shell_NotifyIcon(NIM_DELETE, &nid);
            PostQuitMessage(0);
            break;
        default:
            if (message == WM_TASKBARCREATED) {
                SendMessage(hWnd, WM_CREATE, wParam, lParam);
                break;
            }
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
        case WM_INITDIALOG:
            return (INT_PTR)TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            } else if (LOWORD(wParam) == IDC_ExitButton) {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;
    }
    return (INT_PTR)FALSE;
}
