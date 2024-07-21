#include "framework.h"
#include <cmath>

#include "../Common/Variables.h"
#include "../Common/Helper.h"
#include "../Common/DataTypes.h"
#include "InputDetermine.h"

template <typename TPointer>
void CalculatePosition(TPointer position, POINT& output) {
    RECTSIZE clientSize{};
    GetClientRect(g_hFocusWindow, &clientSize);
    auto realWidth = float(clientSize.height()) * g_currentConfig.AspectRatio.X / g_currentConfig.AspectRatio.Y;
    auto paddingX = (float(clientSize.width()) - realWidth) / 2;
    g_playerPosRaw = { double(position->X), double(position->Y) };
    output.x = lrint(float(position->X) / g_pixelRate + g_pixelOffset.X + paddingX);
    output.y = lrint(float(position->Y) / g_pixelRate + g_pixelOffset.Y);
}

static void CalculatePlayerPos(DWORD address) {
    using enum PointDataType;
    if (g_currentConfig.PosDataType == Int)
        CalculatePosition((IntPoint*)address, g_playerPos);
    else if (g_currentConfig.PosDataType == Float)
        CalculatePosition((FloatPoint*)address, g_playerPos);
    else if (g_currentConfig.PosDataType == Short)
        CalculatePosition((ShortPoint*)address, g_playerPos);
    else if (g_currentConfig.PosDataType == Double)
        CalculatePosition((DoublePoint*)address, g_playerPos);
}

namespace helper = common::helper;

namespace core::inputdetermine {
    GameInput DetermineGameInput() {
        using enum GameInput;
        g_gameInput = NONE;
        g_playerPos = {};
        g_playerPosRaw = {};
        DWORD address{};
        if (g_inputEnabled || g_showImGui) {
            if (g_leftMousePressed) {
                g_gameInput |= USE_BOMB;
            }
            if (g_midMousePressed) {
                g_gameInput |= USE_SPECIAL;
            }
            address = helper::CalculateAddress();
            if (address != 0) {
                CalculatePlayerPos(address);

                auto mousePos = helper::GetPointerPosition();

                if (g_playerPos.x < mousePos.x - 1)
                    g_gameInput |= MOVE_RIGHT;
                else if (g_playerPos.x > mousePos.x + 1)
                    g_gameInput |= MOVE_LEFT;

                if (g_playerPos.y < mousePos.y - 1)
                    g_gameInput |= MOVE_DOWN;
                else if (g_playerPos.y > mousePos.y + 1)
                    g_gameInput |= MOVE_UP;
            }
        }
        if (!g_inputEnabled) {
            return NONE;
        }

        return g_gameInput;
    }
}
