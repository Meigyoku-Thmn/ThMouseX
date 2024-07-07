#include "framework.h"
#include <cmath>
#include <random>

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
    double x_error_WHATEVER = 0.0;
    double y_error_WHATEVER = 0.0;
    long prev_xPSpeed = 1;
    long prev_yPSpeed = 1;
    POINT previous_pos = {-1, -1};
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

                auto xDist = mousePos.x - g_playerPos.x;
                auto yDist = mousePos.y - g_playerPos.y;
                auto xPSpeed = previous_pos.x - g_playerPos.x;
                auto yPSpeed = previous_pos.y - g_playerPos.y;
                if(previous_pos = {-1, -1}){
                    xPSpeed = 0;
                    yPSpeed = 0;
                }
                if(xPSpeed == 0) xPSpeed = prev_xPSpeed;
                if(yPSpeed == 0) yPSpeed = prev_yPSpeed;
                if(xDist < 0) xDist = -xDist;
                if(yDist < 0) yDist = -yDist;
                if(xPSpeed < 0) xPSpeed = -xPSpeed;
                if(yPSpeed < 0) yPSpeed = -yPSpeed;
                auto v = xPSpeed;
                if(yPSpeed > xPSpeed){
                    v = yPSpeed;
                }
                if(xDist >= v || yDist >= v || xDist + yDist > 10)){
                    if(xDist > yDist){
                        if (g_playerPos.x < mousePos.x - 1)
                            g_gameInput |= GameInput::MOVE_RIGHT;
                        else if (g_playerPos.x > mousePos.x + 1)
                            g_gameInput |= GameInput::MOVE_LEFT;
                        double error = (double)yDist / xDist;
                        y_error_WHATEVER += error;
                    }else{
                        if (g_playerPos.y < mousePos.y - 1)
                            g_gameInput |= GameInput::MOVE_DOWN;
                        else if (g_playerPos.y > mousePos.y + 1)
                            g_gameInput |= GameInput::MOVE_UP;
                        double error = (double)xDist / yDist;
                        x_error_WHATEVER += error;
                    }
                    if(y_error_WHATEVER > 1){
                        if (g_playerPos.y < mousePos.y - 1)
                            g_gameInput |= GameInput::MOVE_DOWN;
                        else if (g_playerPos.y > mousePos.y + 1)
                            g_gameInput |= GameInput::MOVE_UP;
                        y_error_WHATEVER -= floor(y_error_WHATEVER);
                    }
                    if(x_error_WHATEVER > 1){
                        if (g_playerPos.x < mousePos.x - 1)
                            g_gameInput |= GameInput::MOVE_RIGHT;
                        else if (g_playerPos.x > mousePos.x + 1)
                            g_gameInput |= GameInput::MOVE_LEFT;
                        x_error_WHATEVER -= floor(x_error_WHATEVER);
                    }
                }
                previous_pos = g_playerPos;
                prev_xPSpeed = xPSpeed;
                prev_yPSpeed = yPSpeed;
                // if (g_playerPos.x < mousePos.x - 1)
                //     g_gameInput |= GameInput::MOVE_RIGHT;
                // else if (g_playerPos.x > mousePos.x + 1)
                //     g_gameInput |= GameInput::MOVE_LEFT;

                // if (g_playerPos.y < mousePos.y - 1)
                //     g_gameInput |= GameInput::MOVE_DOWN;
                // else if (g_playerPos.y > mousePos.y + 1)
                //     g_gameInput |= GameInput::MOVE_UP;
            }
        }
        if (!g_inputEnabled) {
            return GameInput::NONE;
        }

        return g_gameInput;
    }
}
