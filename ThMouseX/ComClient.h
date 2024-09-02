#pragma once
#include <Windows.h>
#import "../ThMouseXServer/server.tlb"
using namespace ThMouseXServer;

namespace core::comclient {
    bool Initialize();
    bool GetGameConfig(PWCHAR processName, GameConfig& gameConfig);
}