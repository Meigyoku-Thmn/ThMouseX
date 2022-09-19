module;

#include "framework.h"
#include "macro.h"
#include <vector>

export module core.messagequeuehook;

import common.minhook;
import common.var;
import common.helper;
import core.var;

using namespace std;

HCURSOR WINAPI _SetCursor(HCURSOR hCursor);
decltype(&_SetCursor) OriSetCursor;
int WINAPI _ShowCursor(BOOL bShow);
decltype(&_ShowCursor) OriShowCursor;

export vector<MHookApiConfig> PeekMessageHookConfig{
    {L"USER32.DLL", "SetCursor", &_SetCursor, (PVOID*)&OriSetCursor},
    {L"USER32.DLL", "ShowCursor", &_ShowCursor, (PVOID*)&OriShowCursor},
};

void NormalizeCursor() {
    while (OriShowCursor(FALSE) >= 0);
    while (OriShowCursor(TRUE) < 0);
    // cursor visibility should be 0 at this point
    OriSetCursor(NULL);
}

HCURSOR WINAPI _SetCursor(HCURSOR hCursor) {
    return NULL;
}

int WINAPI _ShowCursor(BOOL bShow) {
    return bShow == TRUE ? 0 : -1;
}

bool CBTProcInstalled;
LRESULT CALLBACK CBTProc(int code, WPARAM wparam, LPARAM lparam) {
    if (!CBTProcInstalled && core_hookApplied)
        NormalizeCursor();
    CBTProcInstalled = true;
    return CallNextHookEx(NULL, code, wparam, lparam);
}

bool KeyboardProcInstalled;
auto hCursor = LoadCursorA(NULL, IDC_ARROW);
LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam) {
    if (code == HC_ACTION && core_hookApplied) {
        if (wParam == 0x41)
            OriSetCursor(NULL); // A key
        else if (wParam == 0x53)
            OriSetCursor(hCursor); // S key
    }
    KeyboardProcInstalled = true;
    return CallNextHookEx(NULL, code, wParam, lParam);
}

bool MouseProcInstalled;
LRESULT CALLBACK MouseProc(int code, WPARAM wParam, LPARAM lParam) {
    static bool isRightMousePressing = false;
    if (code == HC_ACTION && core_hookApplied) {
        if (wParam == WM_LBUTTONDOWN)
            g_leftMousePressed = true;
        else if (wParam == WM_MBUTTONDOWN)
            g_midMousePressed = true;
        else if (wParam == WM_RBUTTONDOWN)
            isRightMousePressing = true;
        else if (wParam == WM_RBUTTONUP && isRightMousePressing == true) {
            isRightMousePressing = false;
            g_inputEnabled = !g_inputEnabled;
        }
    }
    MouseProcInstalled = true;
    return CallNextHookEx(NULL, code, wParam, lParam);
}

bool CheckHookProcHandle(HHOOK handle) {
    if (handle != NULL)
        return true;
    ReportLastError("Install ThDxHook.dll: Error");
    return false;
}

HHOOK CBTProcHandle;
HHOOK KeyboardProdHandle;
HHOOK MouseProcHandle;

export DLLEXPORT bool InstallThDxHook() {
    // use CBT hook to inject DLL to the target process as soon as possible
    CBTProcHandle = SetWindowsHookExW(WH_CBT, CBTProc, core_hInstance, NULL);
    if (!CheckHookProcHandle(CBTProcHandle))
        return false;
    KeyboardProdHandle = SetWindowsHookExW(WH_KEYBOARD, KeyboardProc, core_hInstance, NULL);
    if (!CheckHookProcHandle(KeyboardProdHandle))
        return false;
    MouseProcHandle = SetWindowsHookExW(WH_MOUSE, MouseProc, core_hInstance, NULL);
    if (!CheckHookProcHandle(MouseProcHandle))
        return false;
    return true;
}

export DLLEXPORT void RemoveThDxHook(void) {
    UnhookWindowsHookEx(CBTProcHandle);
    UnhookWindowsHookEx(KeyboardProdHandle);
    UnhookWindowsHookEx(MouseProcHandle);
    // force all top-level windows to process a message, therefore force all processes to unload the DLL.
    DWORD dwResult;
    SendMessageTimeoutA(HWND_BROADCAST, WM_NULL, 0, 0, SMTO_ABORTIFHUNG | SMTO_NOTIMEOUTIFNOTHUNG, 1000, &dwResult);
}
