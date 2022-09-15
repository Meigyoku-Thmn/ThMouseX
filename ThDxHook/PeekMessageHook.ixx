module;

#include "framework.h"

export module core.peekmesssagehook;

import core.apihijack;
import core.setcursorhook;
import common.var;

typedef BOOL(WINAPI * PeekMessageA_t)(
    LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
typedef BOOL(WINAPI * PeekMessageW_t)(
    LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);

BOOL WINAPI MyPeekMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
BOOL WINAPI MyPeekMessageW(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);

export SDLLHook PeekMessageAHook = {
    .Name = "USER32.DLL",
    .UseDefault = false,
    .DefaultFn = NULL,
    .Functions = {
        {.Name = "PeekMessageA", .HookFn = (DWORD*)MyPeekMessageA},
        {.Name = "PeekMessageW", .HookFn = (DWORD*)MyPeekMessageW},
        {},
    }
};  

HCURSOR _hCursor2 = 0;
void IncreaseCursorVisibility() {
    auto oldFunc = (SetCursor_t)SetCursorHook.Functions[0].OrigFn;
    if (_hCursor2 == 0)
        _hCursor2 = LoadCursor(NULL, IDC_ARROW);
    oldFunc(_hCursor2);
    auto oldFunc2 = (ShowCursor_t)SetCursorHook.Functions[1].OrigFn;
    while (oldFunc2(TRUE) < 0);
}

void DecreaseCursorVisibility() {
    auto oldFunc = (SetCursor_t)SetCursorHook.Functions[0].OrigFn;
    oldFunc(NULL);
    auto oldFunc2 = (ShowCursor_t)SetCursorHook.Functions[1].OrigFn;
    while (oldFunc2(FALSE) >= 0);
}

bool lockCursorSwitch = false;
void ToggleLockCursor() {
    RECT rect;
    GetClientRect(g_hFocusWindow, &rect);
    ClientToScreen(g_hFocusWindow, (POINT*)&rect);
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

void IncreaseCursorSpeed() {}

void DecreaseCursorSpeed() {}

void MyMessageLoop(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg) {
    static BOOL lastR = FALSE;
    if (lpMsg->message == WM_MOVE ||
        lpMsg->message == WM_SYSCOMMAND ||
        lpMsg->message == WM_KILLFOCUS || lpMsg->message == WM_SETFOCUS ||
        lpMsg->message == WM_ACTIVATE || lpMsg->message == WM_NCACTIVATE ||
        lpMsg->message == WM_MOUSEACTIVATE || lpMsg->message == WM_ACTIVATEAPP ||
        lpMsg->message == WM_SYSKEYDOWN) {
        lockCursorSwitch = false;
        //ClipCursor(NULL);
    } else if (lpMsg->message == WM_KEYDOWN) {
        if (lpMsg->wParam == 0x51) {
            //DecreaseCursorSpeed();    // Q key
        } else if (lpMsg->wParam == 0x57) {
            //IncreaseCursorSpeed();    // W key
        } else if (lpMsg->wParam == 0x41) {
            DecreaseCursorVisibility(); // A key
        } else if (lpMsg->wParam == 0x53) {
            IncreaseCursorVisibility(); // S key
        } else if (lpMsg->wParam == 0x44) {
            //ToggleLockCursor();       // D key
        }
    } else if (lpMsg->message == WM_LBUTTONDOWN) {
        g_leftMousePressed = true;
    } else if (lpMsg->message == WM_MBUTTONDOWN) {
        g_midMousePressed = true;
    } else if (lpMsg->message == WM_RBUTTONDOWN) {
        lastR = TRUE;
    } else if (lpMsg->message == WM_RBUTTONUP) {
        if (lastR == TRUE) {
            lastR = FALSE;
            g_inputEnabled = !g_inputEnabled;
        }
    }
}

BOOL WINAPI MyPeekMessageW(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg) {
    auto old_func = (PeekMessageW_t)PeekMessageAHook.Functions[1].OrigFn;
    auto r = old_func(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
    MyMessageLoop(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
    return r;
}

BOOL WINAPI MyPeekMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg) {
    auto old_func = (PeekMessageA_t)PeekMessageAHook.Functions[0].OrigFn;
    auto r = old_func(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
    MyMessageLoop(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
    return r;
}
