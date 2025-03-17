#include <Windows.h>
#include <cmath>
#include <cstdint>

#include "../Common/Variables.h"
#include "../Common/Helper.h"
#include "../Common/DataTypes.h"
#include "InputDetermine.h"

template <typename TPointer>
void CalculatePosition(TPointer position, POINT& output) {
    RECTSIZE clientSize{};
    GetClientRect(g_hFocusWindow, &clientSize);
    auto realWidth = float(clientSize.height()) * g_gameConfig.AspectRatio.X / g_gameConfig.AspectRatio.Y;
    auto paddingX = (float(clientSize.width()) - realWidth) / 2;
    g_playerPosRaw = { double(position->X), double(position->Y) };
    output.x = lrint(float(position->X) / g_pixelRate + g_pixelOffset.X + paddingX);
    output.y = lrint(float(position->Y) / g_pixelRate + g_pixelOffset.Y);
}

static void CalculatePlayerPos(uintptr_t address) {
    if (g_gameConfig.PosDataType == PointDataType_Int)
        CalculatePosition((IntPoint*)address, g_playerPos);
    else if (g_gameConfig.PosDataType == PointDataType_Float)
        CalculatePosition((FloatPoint*)address, g_playerPos);
    else if (g_gameConfig.PosDataType == PointDataType_Short)
        CalculatePosition((ShortPoint*)address, g_playerPos);
    else if (g_gameConfig.PosDataType == PointDataType_Double)
        CalculatePosition((DoublePoint*)address, g_playerPos);
}

namespace helper = common::helper;

struct InputRule {
    bool* btnOnPtr;
    GameInput gameInput;
};

namespace core::inputdetermine {
    using enum GameInput;

    InputRule inputRule[]{
        { &g_leftClicked,       CLICK_LEFT      },
        { &g_middleClicked,     CLICK_MIDDLE    },
        { &g_rightClicked,      CLICK_RIGHT     },
        { &g_xButton1Clicked,   CLICK_XBUTTON1  },
        { &g_xButton2Clicked,   CLICK_XBUTTON2  },
        { &g_scrolledUp,        SCROLL_UP       },
        { &g_scrolledDown,      SCROLL_DOWN     },
        { &g_scrolledLeft,      SCROLL_LEFT     },
        { &g_scrolledRight,     SCROLL_RIGHT    },
    };

    static double x_error_WHATEVER = 0.0;
    static double y_error_WHATEVER = 0.0;
    static long prev_xPSpeed = 1;
    static long prev_yPSpeed = 1;
    static POINT previous_pos = { -1, -1 };

    GameInput DetermineGameInput() {
        g_gameInput = NONE;
        g_playerPos = {};
        g_playerPosRaw = {};
        uintptr_t address{};

        if (!g_inputEnabled)
            return NONE;

        for (const auto& ruleItem : inputRule)
            if (*ruleItem.btnOnPtr)
                g_gameInput |= ruleItem.gameInput;

        address = helper::CalculateAddress();
        if (address == 0)
            return g_gameInput;

        CalculatePlayerPos(address);

        auto mousePos = helper::GetPointerPosition();

        if (g_movementAlgorithm == MovementAlgorithm::Bresenham) {
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
        else if (g_movementAlgorithm == MovementAlgorithm::Simple) {
            if (g_playerPos.x < mousePos.x - 1)
                g_gameInput |= GameInput::MOVE_RIGHT;
            else if (g_playerPos.x > mousePos.x + 1)
                g_gameInput |= GameInput::MOVE_LEFT;
            if (g_playerPos.y < mousePos.y - 1)
                g_gameInput |= GameInput::MOVE_DOWN;
            else if (g_playerPos.y > mousePos.y + 1)
                g_gameInput |= GameInput::MOVE_UP;
        }

        return g_gameInput;
    }
}
