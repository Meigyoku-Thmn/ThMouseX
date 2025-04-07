#pragma once
#include "../Common/macro.h"
#include "../Common/DataTypes.h"
#include <Windows.h>
#include <string>

namespace core::configuration {
    DLLEXPORT_C bool MarkThMouseXProcess();
    DLLEXPORT_C UINT_PTR GetMemBlockSize(LPCVOID address);
    DLLEXPORT_C bool GetGameConfig(LPCWSTR processName, CommonConfig** commonConfig, DWORD* commonConfigSize, GameConfig** gameConfig, DWORD* gameConfigSize);
    DLLEXPORT_C bool ReadGamesFile();
    DLLEXPORT_C bool ReadGeneralConfigFile();
}