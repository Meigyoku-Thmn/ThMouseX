module;

#include "framework.h"
#include "macro.h"

export module core.windowshook;

import common.datatype;
import common.var;

#pragma data_seg(".HOOKDATA") // Shared data among all instances.
HHOOK hHookW = NULL;
#pragma data_seg()
#pragma comment(linker, "/SECTION:.HOOKDATA,RWS")

export HINSTANCE hinstance;

int ReportLastError();
LRESULT CALLBACK hookprocW(int ncode, WPARAM wparam, LPARAM lparam);
DWORD ReadJoyButtonNumber(int buttonNumber, int defaultValue);

export DLLEXPORT int installThDxHook(
    const GameConfigArray *config, int leftButton, int midButton, const char* pTextureFilePath);
export DLLEXPORT int removeThDxHook(void);

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

DWORD ReadJoyButtonNumber(int buttonNumber, int defaultValue) {
    if (buttonNumber >= 0 && buttonNumber < 32)
        return 1 << buttonNumber;
    else
        return defaultValue;
}

int installThDxHook(
    const GameConfigArray* config, int leftButton, int midButton, const char* pTextureFilePath
) {
    strcpy_s<TEXTURE_FILE_PATH_LEN>(gs_textureFilePath, pTextureFilePath);

    // store the configuration into the shared memory section
    gs_gameConfigArray = *config;
    gs_boomButton = ReadJoyButtonNumber(leftButton, 1);
    gs_extraButton = ReadJoyButtonNumber(midButton, 3);

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

int removeThDxHook(void) {
    auto rs = UnhookWindowsHookEx(hHookW);
    DWORD dwResult;
    // force all top-level windows to process a message, therefore force all processes to unload the DLL.
    SendMessageTimeout(HWND_BROADCAST, WM_NULL, 0, 0, SMTO_ABORTIFHUNG | SMTO_NOTIMEOUTIFNOTHUNG, 1000, &dwResult);
    return rs;
}
