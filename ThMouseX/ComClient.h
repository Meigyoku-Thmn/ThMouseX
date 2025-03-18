#pragma once
#include <Windows.h>
#include "../Common/ComServer.h"

namespace core::comclient {
    bool Initialize();
    bool GetGameConfig(const WCHAR* processName, GameConfig& gameConfig, CommonConfig& commonConfig);
}