#include "stdafx.h"
#include "MyMmsystem.h"
#include "global.h"
#include "Line.h"
#include "../DX8Hook/subGlobal.h"

#define X_MIN	0
#define X_MID	32767
#define X_MAX	65535
#define Y_MIN	0
#define Y_MID	32767
#define Y_MAX	65535

enum {
    WinmmFN_joyGetDevCapsA = 0,
    WinmmFN_joyGetPosEx = 1,
    WinmmFN_joyGetPos = 2,
};

SDLLHook WinmmHook = {
    "WINMM.DLL",
    false, NULL,		// Default hook disabled, NULL function pointer.
    {
        {"joyGetDevCapsA", (DWORD*)MyJoyGetDevCapsA},
        {"joyGetPosEx", (DWORD*)MyJoyGetPosEx},
        {"joyGetPos", (DWORD*)MyJoyGetPos},
        {NULL, NULL}
    }
};

typedef MMRESULT(WINAPI *joyGetPos_t)(UINT uJoyID, LPJOYINFO pji);

MMRESULT WINAPI MyJoyGetPos(UINT uJoyID, LPJOYINFO pji) {
    joyGetPos_t old_func = (joyGetPos_t)WinmmHook.Functions[WinmmFN_joyGetPos].OrigFn;
    if (uJoyID != 0)
        return old_func(uJoyID, pji);
    if (keyBoard == TRUE) {
        pji->wXpos = X_MID;
        pji->wYpos = Y_MID;
        return JOYERR_NOERROR;
    }
    if (dxVersion != 8 ? g_working : g_working2) {
        DWORD address = g_currentGameConfig.Posistion.Chain[0];
        if (offsetIsRelative == true) {
            if (firstOffsetDirection == -1) address = baseOfCode - address;
            else address += baseOfCode;
        }
        for (int i = 1; i < g_currentGameConfig.Posistion.Length; i++) {
            address = *((DWORD*)address);
            if (address == 0)
                break;
            address += g_currentGameConfig.Posistion.Chain[i];
        }
        if (address != 0) {
            POINT playerPos, mousePos;
            if (g_currentGameConfig.PosDataType == Int_DataType) {
                IntPoint * pPosition = (IntPoint*)(address);
                playerPos.x =
                    pPosition->X / g_currentGameConfig.PixelRate + g_currentGameConfig.PixelOffset.X;
                playerPos.y =
                    pPosition->Y / g_currentGameConfig.PixelRate + g_currentGameConfig.PixelOffset.Y;
            } else if (g_currentGameConfig.PosDataType == Float_DataType) {
                FloatPoint * pPosition = (FloatPoint*)(address);
                playerPos.x =
                    pPosition->X / g_currentGameConfig.PixelRate + g_currentGameConfig.PixelOffset.X;
                playerPos.y =
                    pPosition->Y / g_currentGameConfig.PixelRate + g_currentGameConfig.PixelOffset.Y;
            } else if (g_currentGameConfig.PosDataType == Short_DataType) {
                ShortPoint * pPosition = (ShortPoint*)(address);
                playerPos.x =
                    pPosition->X / g_currentGameConfig.PixelRate + g_currentGameConfig.PixelOffset.X;
                playerPos.y =
                    pPosition->Y / g_currentGameConfig.PixelRate + g_currentGameConfig.PixelOffset.Y;
            }
            GetCursorPos(&mousePos);
            if ((dxVersion != 8 ? windowed : windowed2) == TRUE)
                ScreenToClient(dxVersion != 8 ? g_hFocusWindow : g_hFocusWindow2, &mousePos);
            auto shiftPressed = GetKeyState(VK_SHIFT) & 0x80;
            shiftPressed = false;
            if (shiftPressed ? playerPos.x < mousePos.x : playerPos.x < mousePos.x - 1) {
                pji->wXpos = X_MAX;
            } else if (shiftPressed ? playerPos.x > mousePos.x : playerPos.x > mousePos.x + 1) {
                pji->wXpos = X_MIN;
            } else {
                pji->wXpos = X_MID;
            }

            if (shiftPressed ? playerPos.y < mousePos.y : playerPos.y < mousePos.y - 1) {
                pji->wYpos = Y_MAX;
            } else if (shiftPressed ? playerPos.y > mousePos.y : playerPos.y > mousePos.y + 1) {
                pji->wYpos = Y_MIN;
            } else {
                pji->wYpos = Y_MID;
            }
        } else {
            pji->wXpos = X_MID;
            pji->wYpos = Y_MID;
        }
    } else {
        pji->wXpos = X_MID;
        pji->wYpos = Y_MID;
    }
    pji->wButtons = 0x00000000;
    return JOYERR_NOERROR;
}


static int g_joy_init = -1;

MMRESULT WINAPI MyJoyGetDevCapsA(__in UINT_PTR uJoyID, __out_bcount(cbjc) LPJOYCAPSA pjc, __in UINT cbjc) {
    g_joy_init = 1;
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

MMRESULT WINAPI MyJoyGetPosEx(__in UINT uJoyID, __out LPJOYINFOEX pji) {
    if (g_joy_init == -1) {
        g_joy_init = 0;
        return JOYERR_NOERROR;
    } else if (g_joy_init == 0) {
        return JOYERR_PARMS;
    }
    if (keyBoard == TRUE) {
        pji->dwXpos = X_MID;
        pji->dwYpos = Y_MID;
        return JOYERR_NOERROR;
    }
    if (dxVersion != 8 ? g_working : g_working2) {
        DWORD address = g_currentGameConfig.Posistion.Chain[0];
        if (offsetIsRelative == true) {
            if (firstOffsetDirection == -1) address = baseOfCode - address;
            else address += baseOfCode;
        }
        for (int i = 1; i < g_currentGameConfig.Posistion.Length; i++) {
            address = *((DWORD*)address);
            if (address == 0)
                break;
            address += g_currentGameConfig.Posistion.Chain[i];
        }
        if (address != 0) {
            POINT playerPos, mousePos;
            if (g_currentGameConfig.PosDataType == Int_DataType) {
                IntPoint * pPosition = (IntPoint*)(address);
                playerPos.x =
                    pPosition->X / g_currentGameConfig.PixelRate + g_currentGameConfig.PixelOffset.X;
                playerPos.y =
                    pPosition->Y / g_currentGameConfig.PixelRate + g_currentGameConfig.PixelOffset.Y;
            } else if (g_currentGameConfig.PosDataType == Float_DataType) {
                FloatPoint * pPosition = (FloatPoint*)(address);
                playerPos.x =
                    pPosition->X / g_currentGameConfig.PixelRate + g_currentGameConfig.PixelOffset.X;
                playerPos.y =
                    pPosition->Y / g_currentGameConfig.PixelRate + g_currentGameConfig.PixelOffset.Y;
            } else if (g_currentGameConfig.PosDataType == Short_DataType) {
                ShortPoint * pPosition = (ShortPoint*)(address);
                playerPos.x =
                    pPosition->X / g_currentGameConfig.PixelRate + g_currentGameConfig.PixelOffset.X;
                playerPos.y =
                    pPosition->Y / g_currentGameConfig.PixelRate + g_currentGameConfig.PixelOffset.Y;
            }
            GetCursorPos(&mousePos);
            if ((dxVersion != 8 ? windowed : windowed2) == TRUE)
                ScreenToClient(dxVersion != 8 ? g_hFocusWindow : g_hFocusWindow2, &mousePos);
            auto shiftPressed = GetKeyState(VK_SHIFT) & 0x80;
            shiftPressed = false;
            if (shiftPressed ? playerPos.x < mousePos.x : playerPos.x < mousePos.x - 1) {
                pji->dwXpos = X_MAX;
            } else if (shiftPressed ? playerPos.x > mousePos.x : playerPos.x > mousePos.x + 1) {
                pji->dwXpos = X_MIN;
            } else {
                pji->dwXpos = X_MID;
            }

            if (shiftPressed ? playerPos.y < mousePos.y : playerPos.y < mousePos.y - 1) {
                pji->dwYpos = Y_MAX;
            } else if (shiftPressed ? playerPos.y > mousePos.y : playerPos.y > mousePos.y + 1) {
                pji->dwYpos = Y_MIN;
            } else {
                pji->dwYpos = Y_MID;
            }
        } else {
            pji->dwXpos = X_MID;
            pji->dwYpos = Y_MID;
        }
    } else {
        pji->dwXpos = X_MID;
        pji->dwYpos = Y_MID;
    }

    pji->dwButtons = 0x00000000;
    if (g_mouseDown) {
        pji->dwButtons |= g_boomButton;
        g_mouseDown = 0;
    }
    if (g_midMouseDown) {
        pji->dwButtons |= g_extraButton;
        g_midMouseDown = 0;
    }

    return JOYERR_NOERROR;
}