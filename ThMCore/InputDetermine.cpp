#include "framework.h"
#include <cmath>

#include "InputDetermine.h"
#include "../Common/Variables.h"
#include "../Common/Helper.h"
#include "../Common/DataTypes.h"

namespace helper = common::helper;

namespace core::inputdetermine {
    DWORD DetermineGameInput() {
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
            auto address = helper::CalculateAddress();
            if (address != 0) {
                // support borderless mode (except the DOT by DOT mode from Touhou 18 which I gave up)
                RECTSIZE clientSize;
                GetClientRect(g_hFocusWindow, &clientSize);
                auto realWidth = clientSize.height() * g_currentConfig.AspectRatio.X / g_currentConfig.AspectRatio.Y;
                auto paddingX = (clientSize.width() - realWidth) / 2;

                POINT playerPos{};
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

                auto mousePos = helper::GetPointerPosition();

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
}
