#include "framework.h"
#include "resource.h"

import common.datatype;
import main.config;
import core.windowshook;

constexpr auto MAX_LOADSTRING = 100;

HINSTANCE   hInst;							// current instance
TCHAR       szTitle[MAX_LOADSTRING];        // The title bar text
TCHAR       szWindowClass[MAX_LOADSTRING];	// the main window class name
TCHAR       szBalloonInfo[MAX_LOADSTRING];

LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	DialogProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    hInst = hInstance;

    auto hMutex = CreateMutex(NULL, TRUE, "ThMouse");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBox(NULL, "ThMouseX is already running.", "ThMouseX", MB_OK | MB_ICONINFORMATION);
        return 1;
    }

    if (!PopulateMethodRVAs())
        return 1;

    if (!ReadGamesFile())
        return 1;

    if (!ReadIniFile())
        return 1;

    if (!InstallThDxHook())
        return 1;

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_THMOUSE, szWindowClass, MAX_LOADSTRING);
    LoadString(hInstance, IDS_BALLOON_INFO, szBalloonInfo, MAX_LOADSTRING);

    WNDCLASSEX wcex{
        .cbSize = sizeof(WNDCLASSEX),
        .style = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = WndProc,
        .cbClsExtra = 0,
        .cbWndExtra = 0,
        .hInstance = hInstance,
        .hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_THMOUSE)),
        .hCursor = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground = HBRUSH(COLOR_WINDOW + 1),
        .lpszMenuName = NULL,
        .lpszClassName = szWindowClass,
        .hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL)),
    };
    RegisterClassEx(&wcex);

    auto hWnd = CreateWindowEx(WS_EX_TOOLWINDOW, szWindowClass, szTitle, WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
    if (!hWnd)
        return 1;
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    RemoveThDxHook();

    return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE: {
            // send a notification signifies that the tool is opened
            NOTIFYICONDATA nid{
                .cbSize = sizeof(NOTIFYICONDATA),
                .hWnd = hWnd,
                .uID = 0,
                .uFlags = NIF_ICON | NIF_MESSAGE | NIF_INFO | NIF_TIP,
                .uCallbackMessage = WM_USER,
                .hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_THMOUSE)),
            };
            lstrcpy(nid.szTip, szTitle);
            lstrcpy(nid.szInfoTitle, szTitle);
            lstrcpy(nid.szInfo, szBalloonInfo);
            Shell_NotifyIcon(NIM_ADD, &nid);
            return 0;
        }
        case WM_USER:
            switch (lParam) {
                // double click on the system tray icon
                case WM_LBUTTONDBLCLK:
                    // if click exit then send close event
                    if (DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), NULL, DialogProc) == IDC_ExitButton)
                        SendMessage(hWnd, WM_CLOSE, wParam, lParam);
                    return 0;
            }
            break;
        case WM_DESTROY: {
            // clean up notification on exiting
            NOTIFYICONDATA nid{
                .cbSize = sizeof(NOTIFYICONDATA),
                .hWnd = hWnd,
                .uID = 0,
            };
            Shell_NotifyIcon(NIM_DELETE, &nid);
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
        case WM_INITDIALOG:
            return TRUE;
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL || LOWORD(wParam) == IDC_ExitButton) {
                EndDialog(hDlg, LOWORD(wParam));
                return TRUE;
            }
            break;
    }
    return FALSE;
}
