#pragma once

#include "apihijack.h"


MMRESULT WINAPI MyJoyGetDevCapsA(
    __in UINT_PTR uJoyID,
    __out_bcount(cbjc) LPJOYCAPSA pjc,
    __in UINT cbjc
);
MMRESULT WINAPI MyJoyGetPosEx(
    __in  UINT uJoyID,
    __out LPJOYINFOEX pji
);
MMRESULT WINAPI MyJoyGetPos(
    UINT uJoyID,
    LPJOYINFO pji

);
//MMRESULT WINAPI MyTimeBeginPeriod( __in UINT uPeriod);
//MMRESULT WINAPI MyTimeEndPeriod( __in UINT uPeriod);
//DWORD WINAPI MyTimeGetTime(void);

extern SDLLHook WinmmHook;
