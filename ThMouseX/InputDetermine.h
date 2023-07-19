#pragma once
#include "framework.h"

namespace core::inputdetermine {
    constexpr auto USE_BOMB = 0b0000'0001;
    constexpr auto USE_SPECIAL = 0b0000'0010;

    constexpr auto MOVE_LEFT = 0b1000'0000;
    constexpr auto MOVE_RIGHT = 0b0100'0000;
    constexpr auto MOVE_UP = 0b0010'0000;
    constexpr auto MOVE_DOWN = 0b0001'0000;

    DWORD DetermineGameInput();
}
