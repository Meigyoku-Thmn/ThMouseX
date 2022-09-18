module;

#include "framework.h"
#include "macro.h"
#include <vector>

export module core.messagequeuehook;

import common.minhook;
import common.var;
import core.var;

using namespace std;

BOOL WINAPI _PeekMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
decltype(&_PeekMessageA) OriPeekMessageA;
BOOL WINAPI _PeekMessageW(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
decltype(&_PeekMessageW) OriPeekMessageW;
HCURSOR WINAPI _SetCursor(HCURSOR hCursor);
decltype(&_SetCursor) OriSetCursor;
int WINAPI _ShowCursor(BOOL bShow);
decltype(&_ShowCursor) OriShowCursor;

export vector<MHookApiConfig> PeekMessageHookConfig{
    {L"USER32.DLL", "PeekMessageA", &_PeekMessageA, (PVOID*)&OriPeekMessageA},
    {L"USER32.DLL", "PeekMessageW", &_PeekMessageW, (PVOID*)&OriPeekMessageW},
    {L"USER32.DLL", "SetCursor", &_SetCursor, (PVOID*)&OriSetCursor},
    {L"USER32.DLL", "ShowCursor", &_ShowCursor, (PVOID*)&OriShowCursor},
};

export void NormalizeCursor() {
    while (OriShowCursor(FALSE) >= 0);
    while (OriShowCursor(TRUE) < 0);
    // cursor visibility should be 0 at this point
    OriSetCursor(NULL);
}

HCURSOR WINAPI _SetCursor(HCURSOR hCursor) {
    return NULL;
}

int WINAPI _ShowCursor(BOOL bShow) {
    if (bShow == TRUE)
        return 0;
    else
        return -1;
}

auto hCursor = LoadCursor(NULL, IDC_ARROW);
void WINAPI ProcessMessage(LPMSG lpMsg) {
    static bool isRightMousePressing = false;
    if (lpMsg->message == WM_KEYDOWN) {
        if (lpMsg->wParam == 0x41)
            OriSetCursor(NULL); // A key
        else if (lpMsg->wParam == 0x53)
            OriSetCursor(hCursor); // S key
    } else if (lpMsg->message == WM_LBUTTONDOWN)
        g_leftMousePressed = true;
    else if (lpMsg->message == WM_MBUTTONDOWN)
        g_midMousePressed = true;
    else if (lpMsg->message == WM_RBUTTONDOWN)
        isRightMousePressing = true;
    else if (lpMsg->message == WM_RBUTTONUP && isRightMousePressing == true) {
        isRightMousePressing = false;
        g_inputEnabled = !g_inputEnabled;
    }
}

BOOL WINAPI _PeekMessageW(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg) {
    auto rs = OriPeekMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
    if (rs == TRUE)
        ProcessMessage(lpMsg);
    return rs;
}

BOOL WINAPI _PeekMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg) {
    auto rs = OriPeekMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
    if (rs == TRUE)
        ProcessMessage(lpMsg);
    return rs;
}