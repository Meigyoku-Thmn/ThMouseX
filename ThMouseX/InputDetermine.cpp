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
    double x_error_WHATEVER = 0.0;
    double y_error_WHATEVER = 0.0;
    long prev_xPSpeed = 1;
    long prev_yPSpeed = 1;
    POINT previous_pos = { -1, -1 };
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

                auto xDist = abs(mousePos.x - g_playerPos.x);
                auto yDist = abs(mousePos.y - g_playerPos.y);

                auto xPSpeed = previous_pos.x == -1 ? 0 : abs(previous_pos.x - g_playerPos.x);
                auto yPSpeed = previous_pos.x == -1 ? 0 : abs(previous_pos.y - g_playerPos.y);
                if (xPSpeed == 0) xPSpeed = prev_xPSpeed;
                if (yPSpeed == 0) yPSpeed = prev_yPSpeed;

                auto pSpeed = yPSpeed > xPSpeed ? yPSpeed : xPSpeed;

                if (xDist > pSpeed + 1 || yDist > pSpeed + 1 || xDist + yDist > 10) {
                    if (xDist > yDist) {
                        if (g_playerPos.x < mousePos.x)
                            g_gameInput |= MOVE_RIGHT;
                        else if (g_playerPos.x > mousePos.x)
                            g_gameInput |= MOVE_LEFT;
                        auto error = (double)yDist / xDist;
                        y_error_WHATEVER += error;
                    }
                    else {
                        if (g_playerPos.y < mousePos.y)
                            g_gameInput |= MOVE_DOWN;
                        else if (g_playerPos.y > mousePos.y)
                            g_gameInput |= MOVE_UP;
                        auto error = (double)xDist / yDist;
                        x_error_WHATEVER += error;
                    }
                    if (y_error_WHATEVER > 1) {
                        if (g_playerPos.y < mousePos.y)
                            g_gameInput |= MOVE_DOWN;
                        else if (g_playerPos.y > mousePos.y)
                            g_gameInput |= MOVE_UP;
                        y_error_WHATEVER -= floor(y_error_WHATEVER);
                    }
                    if (x_error_WHATEVER > 1) {
                        if (g_playerPos.x < mousePos.x)
                            g_gameInput |= MOVE_RIGHT;
                        else if (g_playerPos.x > mousePos.x)
                            g_gameInput |= MOVE_LEFT;
                        x_error_WHATEVER -= floor(x_error_WHATEVER);
                    }
                }

                previous_pos = g_playerPos;
                prev_xPSpeed = xPSpeed;
                prev_yPSpeed = yPSpeed;
            }
        }
        if (!g_inputEnabled) {
            return NONE;
        }

        return g_gameInput;
    }
}
