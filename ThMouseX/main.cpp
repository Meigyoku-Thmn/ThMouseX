#include "framework.h"
#include "resource.h"
#include <clocale>
#include <iostream>

import common.datatype;
import main.config;
import core.messagequeuehook;

namespace config = main::config;
namespace messagequeuehook = core::messagequeuehook;

constexpr auto MAX_LOADSTRING = 100;
constexpr auto NOTIFICATION_ICON_MSG = WM_USER;
constexpr auto SHOW_ABOUT_DIALOG = WM_USER + 1;
constexpr auto EXIT_MSG = WM_USER + 2;

HINSTANCE   hInst;							// current instance
CHAR        szTitle[MAX_LOADSTRING];        // The title bar text
CHAR        szWindowClass[MAX_LOADSTRING];	// the main window class name
CHAR        szBalloonInfo[MAX_LOADSTRING];

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DialogProc(HWND, UINT, WPARAM, LPARAM);
void             ShowContextMenu(HWND hWnd);
void             ShowAboutDialog(HWND hWnd, WPARAM wParam, LPARAM lParam);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    setlocale(LC_ALL, ".UTF8");

    hInst = hInstance;

    auto mutex = CreateMutexA(NULL, TRUE, "ThMouseX");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBoxA(NULL, "ThMouseX is already running.", "ThMouseX", MB_OK | MB_ICONINFORMATION);
        return 1;
    }

    if (!messagequeuehook::TestRegisteredWindowMessages())
        return 1;

    if (!config::PopulateMethodRVAs())
        return 1;

    if (!config::ReadGamesFile())
        return 1;

    if (!config::ReadIniFile())
        return 1;

    if (!messagequeuehook::InstallHooks())
        return 1;

    // Initialize global strings
    LoadStringA(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringA(hInstance, IDC_THMOUSEX, szWindowClass, MAX_LOADSTRING);
    LoadStringA(hInstance, IDS_BALLOON_INFO, szBalloonInfo, MAX_LOADSTRING);

    WNDCLASSEXA wcex{
        .cbSize = sizeof(WNDCLASSEX),
        .style = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = WndProc,
        .cbClsExtra = 0,
        .cbWndExtra = 0,
        .hInstance = hInstance,
        .hIcon = LoadIconA(hInstance, MAKEINTRESOURCEA(IDI_THMOUSEX)),
        .hCursor = LoadCursorA(NULL, IDC_ARROW),
        .hbrBackground = HBRUSH(COLOR_WINDOW + 1),
        .lpszMenuName = NULL,
        .lpszClassName = szWindowClass,
        .hIconSm = LoadIconA(wcex.hInstance, MAKEINTRESOURCEA(IDI_SMALL)),
    };
    RegisterClassExA(&wcex);

    auto hWnd = CreateWindowExA(WS_EX_TOOLWINDOW, szWindowClass, szTitle, WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
    if (!hWnd)
        return 1;
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    messagequeuehook::RemoveHooks();

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
                .uCallbackMessage = NOTIFICATION_ICON_MSG,
                .hIcon = LoadIconA(hInst, MAKEINTRESOURCE(IDI_THMOUSEX)),
            };
            lstrcpyA(nid.szTip, szTitle);
            lstrcpyA(nid.szInfoTitle, szTitle);
            lstrcpyA(nid.szInfo, szBalloonInfo);
            Shell_NotifyIconA(NIM_ADD, &nid);
            return 0;
        }
        case NOTIFICATION_ICON_MSG:
            switch (lParam) {
                // double click on the system tray icon
                case WM_LBUTTONDBLCLK:
                    ShowAboutDialog(hWnd, wParam, lParam);
                    return 0;
                case WM_RBUTTONDOWN:
                case WM_CONTEXTMENU:
                    ShowContextMenu(hWnd);
                    return 0;
            }
            break;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case SHOW_ABOUT_DIALOG:
                    ShowAboutDialog(hWnd, wParam, lParam);
                    return 0;
                case EXIT_MSG:
                    SendMessageA(hWnd, WM_CLOSE, wParam, lParam);
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
            Shell_NotifyIconA(NIM_DELETE, &nid);
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

void ShowContextMenu(HWND hWnd) {
    POINT cursorPos;
    GetCursorPos(&cursorPos);
    auto hMenu = CreatePopupMenu();
    if (hMenu) {
        InsertMenuA(hMenu, -1, MF_BYPOSITION, SHOW_ABOUT_DIALOG, "&About");
        InsertMenuA(hMenu, -1, MF_BYPOSITION, EXIT_MSG, "&Exit");
        SetForegroundWindow(hWnd);
        TrackPopupMenu(hMenu, TPM_BOTTOMALIGN, cursorPos.x, cursorPos.y, 0, hWnd, NULL);
        DestroyMenu(hMenu);
    }
}

void ShowAboutDialog(HWND hWnd, WPARAM wParam, LPARAM lParam) {
    if (DialogBoxA(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), NULL, DialogProc) == IDC_ExitButton)
        SendMessageA(hWnd, WM_CLOSE, wParam, lParam);
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
