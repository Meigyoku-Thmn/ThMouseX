module;

#include "framework.h"
#include "macro.h"

export module core.windowshook;

import common.datatype;
import common.helper;
import common.var;
import core.directx9hook;
import dx8.hook;

// Shared data among all instances.
#pragma data_seg(".HOOKDAT")
HHOOK hHookW = NULL;
#pragma data_seg()
#pragma comment(linker, "/SECTION:.HOOKDAT,RWS")

export HINSTANCE hinstance;

LRESULT CALLBACK hookprocW(int ncode, WPARAM wparam, LPARAM lparam) {
    // TODO: add a way for user to specify window class name
    if (g_hFocusWindow == NULL && ncode == HCBT_CREATEWND) {
        auto hwnd = (HWND)wparam;
        WCHAR buf[256];
        GetClassNameW(hwnd, buf, sizeof(buf) / sizeof(buf[0]));
        if (wcscmp(L"BASE", buf) == 0)
            g_hFocusWindow = hwnd;
    }
    // pass control to next hook in the hook chain.
    return CallNextHookEx(hHookW, ncode, wparam, lparam);
}

export DLLEXPORT bool InstallThDxHook() {
    hHookW = SetWindowsHookExW(WH_CBT, hookprocW, hinstance, NULL);
    if (hHookW == NULL) {
        ReportLastError("Install ThDxHook.dll: Error");
        return false;
    }
    return true;
}

export DLLEXPORT void RemoveThDxHook(void) {
    UnhookWindowsHookEx(hHookW);
    DWORD dwResult;
    // force all top-level windows to process a message, therefore force all processes to unload the DLL.
    SendMessageTimeout(HWND_BROADCAST, WM_NULL, 0, 0, SMTO_ABORTIFHUNG | SMTO_NOTIMEOUTIFNOTHUNG, 1000, &dwResult);
}
