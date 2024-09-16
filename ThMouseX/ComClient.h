#pragma once
#include <Windows.h>
#include "../Common/ComServer.h"

namespace core::comclient {
    bool Initialize();
    bool GetGameConfig(PWCHAR processName, GameConfig& gameConfig, CommonConfig& commonConfig);
}