#pragma once
#include "../Common/macro.h"
#include "../Common/DataTypes.h"
#include <Windows.h>
#include <string>

namespace core::configuration {
    DLLEXPORT_C bool MarkThMouseXProcess();
    DLLEXPORT_C bool GetGameConfig(PCWCHAR processName, GameConfigEx* gameConfig);
    DLLEXPORT_C bool ReadGamesFile();
    DLLEXPORT_C bool ReadGeneralConfigFile();
}