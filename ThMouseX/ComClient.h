#pragma once
#include <Windows.h>
#include "../Common/ComServer.h"
using namespace ThMouseXServer;

namespace core::comclient {
    bool Initialize();
    bool GetGameConfig(PWCHAR processName, GameConfig& gameConfig);
}