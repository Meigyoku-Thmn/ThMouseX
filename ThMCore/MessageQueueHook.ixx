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
import common.neolua;

using namespace std;

UINT CLEAN_MANAGED_DATA = RegisterWindowMessageA("CLEAN_MANAGED_DATA {6BF7C2B8-F245-4781-AA3C-467366CA3551}");
export DLLEXPORT bool TestRegisteredWindowMessages() {
    if (CLEAN_MANAGED_DATA == 0) {
        MessageBoxA(NULL, "Failed to register CLEAN_MANAGED_DATA message.", "RegisterWindowMessage error", MB_OK | MB_ICONERROR);
        return false;
    }
    return true;
}

HCURSOR WINAPI _SetCursor(HCURSOR hCursor);
decltype(&_SetCursor) OriSetCursor;
int WINAPI _ShowCursor(BOOL bShow);
decltype(&_ShowCursor) OriShowCursor;

bool isCursorShow = true;
auto hCursor = LoadCursorA(NULL, IDC_ARROW);

constexpr auto VK_A = 0x41;
constexpr auto VK_S = 0x53;

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

int cursorVisibility;
void ShowCursorEx(bool show) {
    if (show && cursorVisibility < 0)
        OriShowCursor(TRUE);
    else if (!show && cursorVisibility >= 0)
        OriShowCursor(FALSE);
}

void HideMousePointer() {
    OriSetCursor(NULL);
    ShowCursorEx(false);
    isCursorShow = false;
}

void ShowMousePointer() {
    OriSetCursor(hCursor);
    ShowCursorEx(true);
    isCursorShow = true;
}

struct OnInit {
    OnInit() {
        // Hide the mouse cursor when D3D is initialized
        RegisterD3D8InitializeCallback(Callback);
        RegisterD3D9InitializeCallback(Callback);
    }
    static void Callback() {
        HideMousePointer();
    }
} _;

void NormalizeCursor() {
    // Set cursor visibility to -1, reset cursor to a normal arrow,
    // to ensure that there is a visible mouse cursor on the game's config dialog
    while (OriShowCursor(TRUE) < 0);
    while (OriShowCursor(FALSE) >= 0);
    cursorVisibility = -1;
    ShowMousePointer();
}

LRESULT CALLBACK GetMsgProcW(int code, WPARAM wParam, LPARAM lParam) {
    if (code == HC_ACTION && core_hookApplied) {
        static auto neoLuaInitialized = false;
        if (!neoLuaInitialized) {
            neoLuaInitialized = true;
            InitializeNeoLua();
        }
        static auto cursorNormalized = false;
        if (!cursorNormalized) {
            cursorNormalized = true;
            NormalizeCursor();
        }
        static bool isRightMousePressing = false;
        auto e = (PMSG)lParam;
        if (e->message == WM_LBUTTONDOWN)
            g_leftMousePressed = true;
        else if (e->message == WM_MBUTTONDOWN)
            g_midMousePressed = true;
        else if (e->message == WM_RBUTTONDOWN)
            isRightMousePressing = true;
        else if (e->message == WM_RBUTTONUP && isRightMousePressing == true) {
            isRightMousePressing = false;
            g_inputEnabled = !g_inputEnabled;
        } else if (e->message == WM_KEYDOWN) {
            if (e->wParam == VK_A)
                HideMousePointer();
            else if (e->wParam == VK_S)
                ShowMousePointer();
        }
    }
    return CallNextHookEx(NULL, code, wParam, lParam);
}

LRESULT CALLBACK CallWndRetProcW(int code, WPARAM wParam, LPARAM lParam) {
    if (code == HC_ACTION && core_hookApplied) {
        auto e = (PCWPRETSTRUCT)lParam;
        if (e->message == CLEAN_MANAGED_DATA) {
            UninitializeNeoLua();
        }
        else if (e->message == WM_SETCURSOR) {
            if (LOWORD(e->lParam) == HTCLIENT) {
                if (isCursorShow)
                    ShowMousePointer();
                else
                    HideMousePointer();
            } else {
                ShowCursorEx(true);
                DefWindowProcW(e->hwnd, e->message, e->wParam, e->lParam);
            }
        }
    }
    return CallNextHookEx(NULL, code, wParam, lParam);
}

bool CheckHookProcHandle(HHOOK handle) {
    if (handle != NULL)
        return true;
    ReportLastError("ThMouseX: SetWindowsHookEx Error");
    return false;
}

HHOOK GetMsgProcHandle;
HHOOK CallWndRetProcHandle;

export DLLEXPORT bool InstallHooks() {
    GetMsgProcHandle = SetWindowsHookExW(WH_GETMESSAGE, GetMsgProcW, core_hInstance, NULL);
    if (!CheckHookProcHandle(GetMsgProcHandle))
        return false;
    CallWndRetProcHandle = SetWindowsHookExW(WH_CALLWNDPROCRET, CallWndRetProcW, core_hInstance, NULL);
    if (!CheckHookProcHandle(CallWndRetProcHandle))
        return false;
    return true;
}

export DLLEXPORT void RemoveHooks() {
    DWORD _;
    auto broadcastFlags = SMTO_ABORTIFHUNG | SMTO_NOTIMEOUTIFNOTHUNG;
    // notify targets to clean up managed data, but managed DLLs/assemblies unfortunately cannot be unloaded.
    SendMessageTimeoutW(HWND_BROADCAST, CLEAN_MANAGED_DATA, 0, 0, broadcastFlags, 1000, &_);
    // unregister hooks.
    UnhookWindowsHookEx(GetMsgProcHandle);
    UnhookWindowsHookEx(CallWndRetProcHandle);
    // force all top-level windows to process a message, therefore force all processes to unload the DLL.
    SendMessageTimeoutW(HWND_BROADCAST, WM_NULL, 0, 0, broadcastFlags, 1000, &_);
}
