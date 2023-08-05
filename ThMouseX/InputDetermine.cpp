#include "framework.h"
#include <cmath>

#include "../Common/Variables.h"
#include "../Common/Helper.h"
#include "../Common/DataTypes.h"
#include "InputDetermine.h"

template <typename T>
void CalculatePosition(T position, POINT& output) {
    RECTSIZE clientSize{};
    GetClientRect(g_hFocusWindow, &clientSize);
    auto realWidth = clientSize.height() * g_currentConfig.AspectRatio.X / g_currentConfig.AspectRatio.Y;
    auto paddingX = (clientSize.width() - realWidth) / 2;
    g_playerPosRaw = { (double)(position)->X, (double)(position)->Y };
    output.x = lrint((position)->X / g_pixelRate + g_pixelOffset.X + paddingX);
    output.y = lrint((position)->Y / g_pixelRate + g_pixelOffset.Y);
}

void CalculatePlayerPos(DWORD address) {
    if (g_currentConfig.PosDataType == PointDataType::Int)
        CalculatePosition((IntPoint*)address, g_playerPos);
    else if (g_currentConfig.PosDataType == PointDataType::Float)
        CalculatePosition((FloatPoint*)address, g_playerPos);
    else if (g_currentConfig.PosDataType == PointDataType::Short)
        CalculatePosition((ShortPoint*)address, g_playerPos);
    else if (g_currentConfig.PosDataType == PointDataType::Double)
        CalculatePosition((DoublePoint*)address, g_playerPos);
}

namespace helper = common::helper;

namespace core::inputdetermine {
    GameInput DetermineGameInput() {
        g_gameInput = GameInput::NONE;
        g_playerPos = {};
        g_playerPosRaw = {};
        DWORD address{};
        if (g_inputEnabled || g_showImGui) {
            if (g_leftMousePressed) {
                g_gameInput |= GameInput::USE_BOMB;
            }
            if (g_midMousePressed) {
                g_gameInput |= GameInput::USE_SPECIAL;
            }
            address = helper::CalculateAddress();
            if (address != 0) {
                CalculatePlayerPos(address);

                auto mousePos = helper::GetPointerPosition();

                if (g_playerPos.x < mousePos.x - 1)
                    g_gameInput |= GameInput::MOVE_RIGHT;
                else if (g_playerPos.x > mousePos.x + 1)
                    g_gameInput |= GameInput::MOVE_LEFT;

                if (g_playerPos.y < mousePos.y - 1)
                    g_gameInput |= GameInput::MOVE_DOWN;
                else if (g_playerPos.y > mousePos.y + 1)
                    g_gameInput |= GameInput::MOVE_UP;
            }
        }
        if (!g_inputEnabled) {
            return GameInput::NONE;
        }

        return g_gameInput;
    }
}
