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
    if (g_leftMousePressed) {
        gameInput |= USE_BOMB;
        g_leftMousePressed = false;
    }
    if (g_midMousePressed) {
        gameInput |= USE_SPECIAL;
        g_midMousePressed = false;
    }
    if (g_inputEnabled) {
        auto address = g_currentConfig.Address.value();
        if (address != 0) {
            POINT playerPos{}, mousePos;

            auto calulatePosition = [&]<typename T>(T * pPosition) {
                playerPos.x = pPosition->X / g_pixelRate + g_pixelOffset.X;
                playerPos.y = pPosition->Y / g_pixelRate + g_pixelOffset.Y;
            };

            if (g_currentConfig.PosDataType == Int_DataType)
                calulatePosition((IntPoint*)address);
            else if (g_currentConfig.PosDataType == Float_DataType)
                calulatePosition((FloatPoint*)address);
            else if (g_currentConfig.PosDataType == Short_DataType)
                calulatePosition((ShortPoint*)address);

            GetCursorPos(&mousePos);
            if (g_isWindowMode == true)
                ScreenToClient(g_hFocusWindow, &mousePos);

            if (playerPos.x < mousePos.x - 1)
                gameInput |= MOVE_RIGHT;
            else if (playerPos.x > mousePos.x + 1)
                gameInput |= MOVE_LEFT;

            if (playerPos.y < mousePos.y - 1)
                gameInput |= MOVE_DOWN;
            else if (playerPos.y > mousePos.y + 1)
                gameInput |= MOVE_UP;
        }
    }

    return gameInput;
}