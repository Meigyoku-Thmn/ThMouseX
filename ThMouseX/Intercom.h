#pragma once
#include "../Common/macro.h"
#include "../Common/DataTypes.h"
#include <Windows.h>

namespace core::intercom {
    bool QueryGameConfig(LPCWSTR processName, CommonConfig& commonConfig, GameConfig& gameConfig);
}