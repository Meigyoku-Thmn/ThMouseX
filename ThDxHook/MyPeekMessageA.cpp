#include "stdafx.h"
#include "MyPeekMessageA.h"
#include "global.h"
#include "../DX8Hook/subGlobal.h"
#include "MySetCursor.h"

SDLLHook PeekMessageAHook = {
    "USER32.DLL",
    false, NULL,		// Default hook disabled, NULL function pointer.
    {
        {"PeekMessageA", (DWORD*)MyPeekMessageA},
        {"PeekMessageW", (DWORD*)MyPeekMessageW},
        {NULL, NULL}
    }
};

typedef BOOL(WINAPI * PeekMessageA_t)(__out LPMSG lpMsg, __in_opt HWND hWnd, __in UINT wMsgFilterMin, __in UINT wMsgFilterMax, __in UINT wRemoveMsg);
typedef BOOL(WINAPI * PeekMessageW_t)(__out LPMSG lpMsg, __in_opt HWND hWnd, __in UINT wMsgFilterMin, __in UINT wMsgFilterMax, __in UINT wRemoveMsg);

typedef int (WINAPI *ShowCursor_t)(BOOL bShow);
typedef HCURSOR(WINAPI *SetCursor_t)(HCURSOR hCursor);
HCURSOR _hCursor2 = 0;
void IncreaseCursorVisibility() {
    SetCursor_t oldFunc = (SetCursor_t)SetCursorHook.Functions[0].OrigFn;
    if (_hCursor2 == 0)
        _hCursor2 = LoadCursor(NULL, IDC_ARROW);
    oldFunc(_hCursor2);
    ShowCursor_t oldFunc2 = (ShowCursor_t)SetCursorHook.Functions[1].OrigFn;
    while (oldFunc2(TRUE) < 0);
}

void DecreaseCursorVisibility() {
    SetCursor_t oldFunc = (SetCursor_t)SetCursorHook.Functions[0].OrigFn;
    oldFunc(NULL);
    ShowCursor_t oldFunc2 = (ShowCursor_t)SetCursorHook.Functions[1].OrigFn;
    while (oldFunc2(FALSE) >= 0);
}

bool lockCursorSwitch = false;
void ToggleLockCursor() {
    HWND hwnd = dxVersion != 8 ? g_hFocusWindow : g_hFocusWindow2;
    RECT rect = {0};
    GetClientRect(hwnd, &rect);
    ClientToScreen(hwnd, (POINT*)&rect);
    rect.bottom = rect.top + rect.bottom;
    rect.right = rect.left + rect.right;
    if (lockCursorSwitch == false) {
        lockCursorSwitch = true;
        ClipCursor(&rect);
    } else {
        lockCursorSwitch = false;
        ClipCursor(NULL);
    }
}

void IncreaseCursorSpeed() {

}

void DecreaseCursorSpeed() {

}

void MyMessageLoop(__out LPMSG lpMsg, __in_opt HWND hWnd, __in UINT wMsgFilterMin, __in UINT wMsgFilterMax, __in UINT wRemoveMsg) {
    static BOOL lastR = 0;
    if (lpMsg->message == WM_MOVE ||
        lpMsg->message == WM_SYSCOMMAND ||
        lpMsg->message == WM_KILLFOCUS || lpMsg->message == WM_SETFOCUS ||
        lpMsg->message == WM_ACTIVATE || lpMsg->message == WM_NCACTIVATE ||
        lpMsg->message == WM_MOUSEACTIVATE || lpMsg->message == WM_ACTIVATEAPP ||
        lpMsg->message == WM_SYSKEYDOWN) {
        lockCursorSwitch = false;
        //ClipCursor(NULL);
    } else if (lpMsg->message == WM_KEYDOWN) {
        if (lpMsg->wParam == 0x51) // Q key
        {
            //DecreaseCursorSpeed();
        } else if (lpMsg->wParam == 0x57) // W key
        {
            //IncreaseCursorSpeed();
        } else if (lpMsg->wParam == 0x41) // A key
        {
            DecreaseCursorVisibility();
        } else if (lpMsg->wParam == 0x53) // S key
        {
            IncreaseCursorVisibility();
        } else if (lpMsg->wParam == 0x44) // D key
        {
            //ToggleLockCursor();
        }
    } else if (lpMsg->message == WM_LBUTTONDOWN) {
        g_mouseDown = 1;
    } else if (lpMsg->message == WM_MBUTTONDOWN) {
        g_midMouseDown = 1;
    } else if (lpMsg->message == WM_RBUTTONDOWN) {
        lastR = 1;
    } else if (lpMsg->message == WM_RBUTTONUP) {
        if (lastR == 1) {
            lastR = 0;
            if (dxVersion != 8)
                g_working = !g_working;
            else
                g_working2 = !g_working2;
        }
    }
}

BOOL WINAPI MyPeekMessageW(__out LPMSG lpMsg, __in_opt HWND hWnd, __in UINT wMsgFilterMin, __in UINT wMsgFilterMax, __in UINT wRemoveMsg) {
    PeekMessageW_t old_func = (PeekMessageW_t)PeekMessageAHook.Functions[1].OrigFn;
    BOOL r = old_func(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
    MyMessageLoop(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
    return r;
}

BOOL WINAPI MyPeekMessageA(__out LPMSG lpMsg, __in_opt HWND hWnd, __in UINT wMsgFilterMin, __in UINT wMsgFilterMax, __in UINT wRemoveMsg) {
    PeekMessageA_t old_func = (PeekMessageA_t)PeekMessageAHook.Functions[0].OrigFn;
    BOOL r = old_func(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
    MyMessageLoop(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
    return r;
}
