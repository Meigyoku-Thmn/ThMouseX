#include "stdafx.h"
#include "MyKeyboardState.h"
#include "global.h"
#include "Line.h"
#include "../DX8Hook/subGlobal.h"

OriGetKeyboardState getKeyboardState;

#define VK_X 0x58
#define VK_C 0x43

BOOL WINAPI MyGetKeyboardState(PBYTE lpKeyState) {
    BOOL rs = getKeyboardState(lpKeyState);

    if (keyBoard != TRUE && rs != 0) {
        if (g_mouseDown) {
            lpKeyState[VK_X] |= 0x80;
            g_mouseDown = 0;
        }
        if (g_midMouseDown) {
            lpKeyState[VK_C] |= 0x80;
            g_midMouseDown = 0;
        }
        if (dxVersion != 8 ? g_working : g_working2) {
            DWORD address = g_currentGameConfig.Posistion.Chain[0];
            if (offsetIsRelative == true) {
                if (firstOffsetDirection == -1) address = baseOfCode - address;
                else address += baseOfCode;
            }
            for (int i = 1; i < g_currentGameConfig.Posistion.Length; i++) {
                address = *((DWORD*)address);
                if (address == 0) {
                    break;
                }
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

                auto shiftPressed = lpKeyState[VK_SHIFT] & 0x80;
                shiftPressed = false;
                if (shiftPressed ? playerPos.x < mousePos.x : playerPos.x < mousePos.x - 1) {
                    lpKeyState[VK_RIGHT] |= 0x80;
                } else if (shiftPressed ? playerPos.x > mousePos.x : playerPos.x > mousePos.x + 1) {
                    lpKeyState[VK_LEFT] |= 0x80;
                }

                if (shiftPressed ? playerPos.y < mousePos.y : playerPos.y < mousePos.y - 1) {
                    lpKeyState[VK_DOWN] |= 0x80;
                } else if (shiftPressed ? playerPos.y > mousePos.y : playerPos.y > mousePos.y + 1) {
                    lpKeyState[VK_UP] |= 0x80;
                }

            }
        }
    }

    return rs;
}
