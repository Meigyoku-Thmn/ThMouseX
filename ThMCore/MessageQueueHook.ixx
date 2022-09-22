module;

#include "framework.h"
#include "macro.h"
#include <vector>

export module core.messagequeuehook;

import common.minhook;
import common.var;
import common.helper;
import core.var;
import core.directx9hook;
import dx8.hook;

using namespace std;

HCURSOR WINAPI _SetCursor(HCURSOR hCursor);
decltype(&_SetCursor) OriSetCursor;
int WINAPI _ShowCursor(BOOL bShow);
decltype(&_ShowCursor) OriShowCursor;

export vector<MHookApiConfig> MessageQueueHookConfig{
    {L"USER32.DLL", "SetCursor", &_SetCursor, (PVOID*)&OriSetCursor},
    {L"USER32.DLL", "ShowCursor", &_ShowCursor, (PVOID*)&OriShowCursor},
};

HCURSOR WINAPI _SetCursor(HCURSOR hCursor) {
    return NULL;
}

int WINAPI _ShowCursor(BOOL bShow) {
    return bShow == TRUE ? 0 : -1;
}

bool isCursorShow;
auto hCursor = LoadCursorA(NULL, IDC_ARROW);

void NormalizeCursor() {
    // Set cursor visibility to 0, reset cursor to a normal arrow,
    // to ensure that there is a visible mouse cursor on the game's config dialog
    while (OriShowCursor(FALSE) >= 0);
    while (OriShowCursor(TRUE) < 0);
    OriSetCursor(hCursor);
    isCursorShow = true;
}

struct OnInit {
    OnInit() {
        // Hide the mouse cursor when D3D is initialized
        RegisterD3D8InitializeCallback(Callback);
        RegisterD3D9InitializeCallback(Callback);
    }
    static void Callback() {
        OriSetCursor(NULL);
        OriShowCursor(FALSE);
        isCursorShow = false;
    }
} _;

bool CBTProcInstalled;
LRESULT CALLBACK CBTProcW(int code, WPARAM wparam, LPARAM lparam) {
    if (!CBTProcInstalled && core_hookApplied)
        NormalizeCursor();
    CBTProcInstalled = true;
    return CallNextHookEx(NULL, code, wparam, lparam);
}

constexpr auto VK_A = 0x41;
constexpr auto VK_S = 0x53;

bool KeyboardProcInstalled;
LRESULT CALLBACK KeyboardProcW(int code, WPARAM wParam, LPARAM lParam) {
    if (code == HC_ACTION && core_hookApplied) {
        if (wParam == VK_A) {
            // A key
            OriSetCursor(NULL);
            if (isCursorShow)
                OriShowCursor(FALSE);
            isCursorShow = false;
        }
        else if (wParam == VK_S) {
            // S key
            OriSetCursor(hCursor);
            if (!isCursorShow)
                OriShowCursor(TRUE);
            isCursorShow = true;
        }
    }
    KeyboardProcInstalled = true;
    return CallNextHookEx(NULL, code, wParam, lParam);
}

bool MouseProcInstalled;
LRESULT CALLBACK MouseProcW(int code, WPARAM wParam, LPARAM lParam) {
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
    ReportLastError("Install Hook: Error");
    return false;
}

HHOOK CBTProcHandle;
HHOOK KeyboardProdHandle;
HHOOK MouseProcHandle;

export DLLEXPORT bool InstallHooks() {
    // use CBT hook to inject DLL to the target process as soon as possible
    CBTProcHandle = SetWindowsHookExW(WH_CBT, CBTProcW, core_hInstance, NULL);
    if (!CheckHookProcHandle(CBTProcHandle))
        return false;
    KeyboardProdHandle = SetWindowsHookExW(WH_KEYBOARD, KeyboardProcW, core_hInstance, NULL);
    if (!CheckHookProcHandle(KeyboardProdHandle))
        return false;
    MouseProcHandle = SetWindowsHookExW(WH_MOUSE, MouseProcW, core_hInstance, NULL);
    if (!CheckHookProcHandle(MouseProcHandle))
        return false;
    return true;
}

export DLLEXPORT void RemoveHooks(void) {
    UnhookWindowsHookEx(CBTProcHandle);
    UnhookWindowsHookEx(KeyboardProdHandle);
    UnhookWindowsHookEx(MouseProcHandle);
    // force all top-level windows to process a message, therefore force all processes to unload the DLL.
    DWORD dwResult;
    SendMessageTimeoutA(HWND_BROADCAST, WM_NULL, 0, 0, SMTO_ABORTIFHUNG | SMTO_NOTIMEOUTIFNOTHUNG, 1000, &dwResult);
}
