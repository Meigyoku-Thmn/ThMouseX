#pragma once

#include "apihijack.h"

BOOL WINAPI MyPeekMessageA(
    __out LPMSG lpMsg,
    __in_opt HWND hWnd,
    __in UINT wMsgFilterMin,
    __in UINT wMsgFilterMax,
    __in UINT wRemoveMsg
);

BOOL WINAPI MyPeekMessageW(
    __out LPMSG lpMsg,
    __in_opt HWND hWnd,
    __in UINT wMsgFilterMin,
    __in UINT wMsgFilterMax,
    __in UINT wRemoveMsg
);

extern SDLLHook PeekMessageAHook;
