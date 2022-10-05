module;

#include "framework.h"
#include <cmath>

export module core.inputdetermine;

import common.var;
import common.helper;
import common.datatype;

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
        auto address = CalculateAddress();
        if (address != 0) {
            // support borderless mode (except the DOT by DOT mode from Touhou 18 which I gave up)
            RECTSIZE clientSize;
            GetClientRect(g_hFocusWindow, &clientSize);
            auto realWidth = clientSize.height() * g_currentConfig.AspectRatio.X / g_currentConfig.AspectRatio.Y;
            auto paddingX = (clientSize.width() - realWidth) / 2;

            POINT playerPos;
#define CalculatePosition(position) { \
    playerPos.x = lrint((position)->X / g_pixelRate + g_pixelOffset.X + paddingX); \
    playerPos.y = lrint((position)->Y / g_pixelRate + g_pixelOffset.Y); \
}
            if (g_currentConfig.PosDataType == PointDataType::Int)
                CalculatePosition((IntPoint*)address)
            else if (g_currentConfig.PosDataType == PointDataType::Float)
                CalculatePosition((FloatPoint*)address)
            else if (g_currentConfig.PosDataType == PointDataType::Short)
                CalculatePosition((ShortPoint*)address);

            auto mousePos = GetPointerPosition();

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