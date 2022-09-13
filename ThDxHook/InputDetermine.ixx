module;

#include "framework.h"

export module core.inputdeterminte;

import common.var;

export constexpr auto USE_BOMB = 0b0000'0001;
export constexpr auto USE_SPECIAL = 0b0000'0010;

export constexpr auto MOVE_LEFT = 0b1000'0000;
export constexpr auto MOVE_RIGHT = 0b0100'0000;
export constexpr auto MOVE_UP = 0b0010'0000;
export constexpr auto MOVE_DOWN = 0b0001'0000;

export DWORD DetermineGameInput() {
    DWORD gameInput = 0;
    if (g_mouseDown) {
        gameInput |= USE_BOMB;
        g_mouseDown = 0;
    }
    if (g_midMouseDown) {
        gameInput |= USE_SPECIAL;
        g_midMouseDown = 0;
    }
    if (g_dxVersion != 8 ? g_working : g_working2) {
        auto address = g_currentGameConfig.Posistion.Chain[0];
        if (g_offsetIsRelative == true) {
            if (g_firstOffsetDirection == -1)
                address = g_baseOfCode - address;
            else
                address += g_baseOfCode;
        }
        for (int i = 1; i < g_currentGameConfig.Posistion.Length; i++) {
            address = *((DWORD*)address);
            if (address == 0)
                break;
            address += g_currentGameConfig.Posistion.Chain[i];
        }
        if (address != 0) {
            POINT playerPos{}, mousePos;

            auto calulatePosition = [&]<typename T>(T * pPosition) {
                playerPos.x = pPosition->X / g_currentGameConfig.PixelRate + g_currentGameConfig.PixelOffset.X;
                playerPos.y = pPosition->Y / g_currentGameConfig.PixelRate + g_currentGameConfig.PixelOffset.Y;
            };

            if (g_currentGameConfig.PosDataType == Int_DataType)
                calulatePosition((IntPoint*)address);
            else if (g_currentGameConfig.PosDataType == Float_DataType)
                calulatePosition((FloatPoint*)address);
            else if (g_currentGameConfig.PosDataType == Short_DataType)
                calulatePosition((ShortPoint*)address);

            GetCursorPos(&mousePos);
            if ((g_dxVersion != 8 ? g_windowed : g_windowed2) == true)
                ScreenToClient(g_dxVersion != 8 ? g_hFocusWindow : g_hFocusWindow2, &mousePos);

            if (g_useAccurateMousePosition ? playerPos.x < mousePos.x : playerPos.x < mousePos.x - 1)
                gameInput |= MOVE_RIGHT;
            else if (g_useAccurateMousePosition ? playerPos.x > mousePos.x : playerPos.x > mousePos.x + 1)
                gameInput |= MOVE_LEFT;

            if (g_useAccurateMousePosition ? playerPos.y < mousePos.y : playerPos.y < mousePos.y - 1)
                gameInput |= MOVE_DOWN;
            else if (g_useAccurateMousePosition ? playerPos.y > mousePos.y : playerPos.y > mousePos.y + 1)
                gameInput |= MOVE_UP;
        }
    }

    return gameInput;
}