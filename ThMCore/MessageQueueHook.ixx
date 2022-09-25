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

bool isCursorShow = true;
auto hCursor = LoadCursorA(NULL, IDC_ARROW);

constexpr auto VK_A = 0x41;
constexpr auto VK_S = 0x53;

export vector<MHookApiConfig> MessageQueueHookConfig{
    {L"USER32.DLL", "SetCursor", &_SetCursor, (PVOID*)&OriSetCursor},
};

HCURSOR WINAPI _SetCursor(HCURSOR hCursor) {
    return NULL;
}

void HideMousePointer() {
    OriSetCursor(NULL);
    isCursorShow = false;
}

void ShowMousePointer() {
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
        HideMousePointer();
    }
} _;

LRESULT CALLBACK CBTProcW(int code, WPARAM wparam, LPARAM lparam) {
    return CallNextHookEx(NULL, code, wparam, lparam);
}

LRESULT CALLBACK GetMsgProcW(int code, WPARAM wParam, LPARAM lParam) {
    if (code == HC_ACTION && core_hookApplied) {
        static bool isRightMousePressing = false;
        auto _ = (PMSG)lParam;
        if (_->message == WM_LBUTTONDOWN)
            g_leftMousePressed = true;
        else if (_->message == WM_MBUTTONDOWN)
            g_midMousePressed = true;
        else if (_->message == WM_RBUTTONDOWN)
            isRightMousePressing = true;
        else if (_->message == WM_RBUTTONUP && isRightMousePressing == true) {
            isRightMousePressing = false;
            g_inputEnabled = !g_inputEnabled;
        } else if (_->message == WM_KEYDOWN) {
            if (_->wParam == VK_A)
                HideMousePointer();
            else if (_->wParam == VK_S)
                ShowMousePointer();
        }
    }
    return CallNextHookEx(NULL, code, wParam, lParam);
}

LRESULT CALLBACK CallWndRetProcW(int code, WPARAM wparam, LPARAM lparam) {
    if (code == HC_ACTION && core_hookApplied) {
        auto _ = (PCWPRETSTRUCT)lparam;
        if (_->message == WM_SETCURSOR) {
            if (LOWORD(_->lParam) == HTCLIENT) {
                if (isCursorShow)
                    ShowMousePointer();
                else
                    HideMousePointer();
            }
            else {
                OriSetCursor(hCursor);
            }
        }
    }
    return CallNextHookEx(NULL, code, wparam, lparam);
}

bool CheckHookProcHandle(HHOOK handle) {
    if (handle != NULL)
        return true;
    ReportLastError("Install Hook: Error");
    return false;
}

HHOOK CBTProcHandle;
HHOOK GetMsgProcHandle;
HHOOK CallWndRetProcHandle;

export DLLEXPORT bool InstallHooks() {
    // use CBT hook to inject DLL to the target process as soon as possible
    CBTProcHandle = SetWindowsHookExW(WH_CBT, CBTProcW, core_hInstance, NULL);
    if (!CheckHookProcHandle(CBTProcHandle))
        return false;
    GetMsgProcHandle = SetWindowsHookExW(WH_GETMESSAGE, GetMsgProcW, core_hInstance, NULL);
    if (!CheckHookProcHandle(GetMsgProcHandle))
        return false;
    CallWndRetProcHandle = SetWindowsHookExW(WH_CALLWNDPROCRET, CallWndRetProcW, core_hInstance, NULL);
    if (!CheckHookProcHandle(CallWndRetProcHandle))
        return false;
    return true;
}

export DLLEXPORT void RemoveHooks(void) {
    UnhookWindowsHookEx(CBTProcHandle);
    UnhookWindowsHookEx(GetMsgProcHandle);
    UnhookWindowsHookEx(CallWndRetProcHandle);
    // force all top-level windows to process a message, therefore force all processes to unload the DLL.
    DWORD dwResult;
    SendMessageTimeoutA(HWND_BROADCAST, WM_NULL, 0, 0, SMTO_ABORTIFHUNG | SMTO_NOTIMEOUTIFNOTHUNG, 1000, &dwResult);
}
