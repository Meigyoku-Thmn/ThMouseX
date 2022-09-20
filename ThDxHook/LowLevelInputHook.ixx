module;

#include "framework.h"
#include <mmsystem.h>
#include <vector>

export module core.lowlevelinputhook;

import common.minhook;
import core.inputdeterminte;
import common.var;

using namespace std;

constexpr auto X_MIN = 0;
constexpr auto X_MID = 32767;
constexpr auto X_MAX = 65535;
constexpr auto Y_MIN = 0;
constexpr auto Y_MID = 32767;
constexpr auto Y_MAX = 65535;

constexpr auto VK_X = 0x58;
constexpr auto VK_C = 0x43;

BOOL WINAPI _GetKeyboardState(PBYTE lpKeyState);
decltype(&_GetKeyboardState) OriGetKeyboardState;
MMRESULT WINAPI _JoyGetDevCapsA(UINT_PTR uJoyID, LPJOYCAPSA pjc, UINT cbjc);
decltype(&_JoyGetDevCapsA) OriJoyGetDevCapsA;
MMRESULT WINAPI _JoyGetPosEx(UINT uJoyID, LPJOYINFOEX pji);
decltype(&_JoyGetPosEx) OriJoyGetPosEx;
MMRESULT WINAPI _JoyGetPos(UINT uJoyID, LPJOYINFO pji);
decltype(&_JoyGetPos) OriJoyGetPos;

export vector<MHookApiConfig> LowLevelInputHookConfig{
    {L"USER32.DLL", "GetKeyboardState", &_GetKeyboardState, (PVOID*)&OriGetKeyboardState},
    {L"WINMM.DLL", "joyGetDevCapsA", &_JoyGetDevCapsA, (PVOID*)&OriJoyGetDevCapsA},
    {L"WINMM.DLL", "joyGetPosEx", &_JoyGetPosEx, (PVOID*)&OriJoyGetPosEx},
    {L"WINMM.DLL", "joyGetPos", &_JoyGetPos, (PVOID*)&OriJoyGetPos},
};

BOOL WINAPI _GetKeyboardState(PBYTE lpKeyState) {
    auto rs = OriGetKeyboardState(lpKeyState);
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

bool joyPadInitialized = false;

MMRESULT WINAPI _JoyGetDevCapsA(UINT_PTR uJoyID, LPJOYCAPSA pjc, UINT cbjc) {
    joyPadInitialized = true;
    strcpy(pjc->szPname, "ThMouseX");
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

MMRESULT WINAPI _JoyGetPosEx(UINT uJoyID, LPJOYINFOEX pji) {
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

MMRESULT WINAPI _JoyGetPos(UINT uJoyID, LPJOYINFO pji) {
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