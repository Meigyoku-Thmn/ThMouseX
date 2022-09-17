module;

#include "framework.h"
#include <mmsystem.h>

export module core.lowlevelinputhook;

import core.apihijack;
import core.inputdeterminte;
import common.var;

BOOL WINAPI MyGetKeyboardState(PBYTE lpKeyState);
MMRESULT WINAPI MyJoyGetDevCapsA(UINT_PTR uJoyID, LPJOYCAPSA pjc, UINT cbjc);
MMRESULT WINAPI MyJoyGetPosEx(UINT uJoyID, LPJOYINFOEX pji);
MMRESULT WINAPI MyJoyGetPos(UINT uJoyID, LPJOYINFO pji);

constexpr auto X_MIN = 0;
constexpr auto X_MID = 32767;
constexpr auto X_MAX = 65535;
constexpr auto Y_MIN = 0;
constexpr auto Y_MID = 32767;
constexpr auto Y_MAX = 65535;

constexpr auto VK_X = 0x58;
constexpr auto VK_C = 0x43;

enum {
    User32FN_GetKeyboardState = 0,
};
export SDLLHook User32Hook = {
    .Name = "USER32.DLL",
    .UseDefault = false,
    .DefaultFn = NULL,
    .Functions = {
        {.Name = "GetKeyboardState", .HookFn = (DWORD*)MyGetKeyboardState},
        {},
    },
};
using OriGetKeyboardState = BOOL(WINAPI*)(PBYTE lpKeyState);

BOOL WINAPI MyGetKeyboardState(PBYTE lpKeyState) {
    auto old_func = (OriGetKeyboardState)User32Hook.Functions[User32FN_GetKeyboardState].OrigFn;
    auto rs = old_func(lpKeyState);
    if (g_handledByDirectInput != true && rs != 0) {
        g_handledByGetKeyboardState = true;
        auto gameInput = DetermineGameInput();
        if (gameInput & USE_BOMB)
            lpKeyState[VK_X] |= 0x80;
        if (gameInput & USE_SPECIAL)
            lpKeyState[VK_C] |= 0x80;
        if (gameInput & MOVE_LEFT)
            lpKeyState[VK_LEFT] |= 0x80;
        if (gameInput & MOVE_RIGHT)
            lpKeyState[VK_RIGHT] |= 0x80;
        if (gameInput & MOVE_UP)
            lpKeyState[VK_UP] |= 0x80;
        if (gameInput & MOVE_DOWN)
            lpKeyState[VK_DOWN] |= 0x80;
    }
    return rs;
}

enum {
    WinmmFN_joyGetDevCapsA = 0,
    WinmmFN_joyGetPosEx = 1,
    WinmmFN_joyGetPos = 2,
};
export SDLLHook WinmmHook = {
    .Name = "WINMM.DLL",
    .UseDefault = false,
    .DefaultFn = NULL,
    .Functions = {
        {.Name = "joyGetDevCapsA", .HookFn = (DWORD*)MyJoyGetDevCapsA},
        {.Name = "joyGetPosEx", .HookFn = (DWORD*)MyJoyGetPosEx},
        {.Name = "joyGetPos", .HookFn = (DWORD*)MyJoyGetPos},
        {},
    },
};

bool joyPadInitialized = false;

MMRESULT WINAPI MyJoyGetDevCapsA(UINT_PTR uJoyID, LPJOYCAPSA pjc, UINT cbjc) {
    joyPadInitialized = true;
    strcpy(pjc->szPname, "ThMouse");
    pjc->wNumButtons = 32;
    pjc->wMaxButtons = 32;
    pjc->wNumAxes = 2;
    pjc->wMaxAxes = 2;
    pjc->wXmin = X_MIN;
    pjc->wXmax = X_MAX;
    pjc->wYmin = Y_MIN;
    pjc->wYmax = Y_MAX;
    return JOYERR_NOERROR;
}

MMRESULT WINAPI MyJoyGetPosEx(UINT uJoyID, LPJOYINFOEX pji) {
    if (joyPadInitialized == false)
        return JOYERR_NOERROR;
    pji->dwXpos = X_MID;
    pji->dwYpos = Y_MID;
    pji->dwButtons = 0x00000000;
    if (g_handledByDirectInput != true && g_handledByGetKeyboardState != true) {
        g_handledByJoyGetPosEx = true;
        auto gameInput = DetermineGameInput();
        if (gameInput & USE_BOMB)
            pji->dwButtons |= gs_boomButton;
        if (gameInput & USE_SPECIAL)
            pji->dwButtons |= gs_extraButton;
        if (gameInput & MOVE_LEFT)
            pji->dwXpos = X_MIN;
        if (gameInput & MOVE_RIGHT)
            pji->dwXpos = X_MAX;
        if (gameInput & MOVE_UP)
            pji->dwYpos = Y_MIN;
        if (gameInput & MOVE_DOWN)
            pji->dwYpos = Y_MAX;
    }
    return JOYERR_NOERROR;
}

MMRESULT WINAPI MyJoyGetPos(UINT uJoyID, LPJOYINFO pji) {
    if (joyPadInitialized == false)
        return JOYERR_NOERROR;
    pji->wXpos = X_MID;
    pji->wYpos = Y_MID;
    pji->wButtons = 0x00000000;
    if (g_handledByDirectInput != true && g_handledByGetKeyboardState != true && g_handledByJoyGetPosEx != true) {
        auto gameInput = DetermineGameInput();
        if (gameInput & USE_BOMB)
            pji->wButtons |= gs_boomButton;
        if (gameInput & USE_SPECIAL)
            pji->wButtons |= gs_extraButton;
        if (gameInput & MOVE_LEFT)
            pji->wXpos = X_MIN;
        if (gameInput & MOVE_RIGHT)
            pji->wXpos = X_MAX;
        if (gameInput & MOVE_UP)
            pji->wYpos = Y_MIN;
        if (gameInput & MOVE_DOWN)
            pji->wYpos = Y_MAX;
    }
    return JOYERR_NOERROR;
}