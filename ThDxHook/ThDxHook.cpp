// ThDxHook.cpp : Defines the exported functions for the DLL application.

#include "stdafx.h"
#include "ThDxHook.h"
#include "global.h"
#include "apihijack.h"
#include "../DX8Hook/subGlobal.h"

#pragma data_seg(".HOOKDATA") // Shared data among all instances.
HHOOK hHookW = NULL;
#pragma data_seg()
#pragma comment(linker, "/SECTION:.HOOKDATA,RWS") // linker directive

extern HINSTANCE hinstance;

int ReportLastError();
LRESULT CALLBACK hookprocW(int ncode, WPARAM wparam, LPARAM lparam);
void EncodeJoyButton(int buttonNumber, DWORD& joyButton, int defaultValue);

bool hooked = false;
LRESULT CALLBACK hookprocW(int ncode, WPARAM wparam, LPARAM lparam) {
    if (hooked == false && ncode == HCBT_CREATEWND) {
        auto hwnd = (HWND)wparam;
        WCHAR buf[256];
        GetClassNameW(hwnd, buf, 256);
        if (wcscmp(L"NP2-MainWindow", buf) == 0 || wcscmp(L"BASE", buf) == 0) {
            g_hFocusWindow = hwnd;
            g_hFocusWindow2 = hwnd;
            hooked = true;
        }
    }
    return CallNextHookEx(hHookW, ncode, wparam, lparam); // pass control to next hook in the hook chain.
}

void EncodeJoyButton(int buttonNumber, DWORD& joyButton, int defaultValue) {
    if (buttonNumber >= 0 && buttonNumber < 32)
        joyButton = 1 << buttonNumber;
    else
        joyButton = defaultValue;
}

THDXHOOK_API int installThDxHook(
    const GameConfigArray* config, int leftButton, int midButton, const char* pTextureFilePath
) {
    strcpy_s<TEXTURE_FILE_PATH_LEN>(gs_textureFilePath, pTextureFilePath);

    gs_gameConfigArray = *config;
    EncodeJoyButton(leftButton, _ref gs_boomButton, 0x00000001);
    EncodeJoyButton(midButton, _ref gs_extraButton, 0x00000003);

    hHookW = SetWindowsHookExW(WH_CBT, hookprocW, hinstance, NULL);
    if (hHookW == NULL)
        return ReportLastError();
    return 1;
}

int ReportLastError() {
    DWORD dwErr = GetLastError();
    // lookup error code and display it
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dwErr,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR)&lpMsgBuf,
        0,
        NULL);
    MessageBox(NULL, (LPCTSTR)lpMsgBuf, "Install ThDxHook.dll: Error", MB_OK | MB_ICONINFORMATION);
    // Free the buffer.
    LocalFree(lpMsgBuf);

    return 0;
}

THDXHOOK_API int removeThDxHook(void) {
    BOOL rs = UnhookWindowsHookEx(hHookW);
    DWORD dwResult;
    SendMessageTimeout(HWND_BROADCAST, WM_NULL, 0, 0, SMTO_ABORTIFHUNG | SMTO_NOTIMEOUTIFNOTHUNG, 1000, &dwResult);
    return rs;
}
