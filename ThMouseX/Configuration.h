#pragma once
#include "../Common/macro.h"
#include "framework.h"

namespace core::configuration {
    DLLEXPORT_C bool MarkThMouseXProcess();
    DLLEXPORT_C bool ReadGamesFile();
    DLLEXPORT_C bool ReadGeneralConfigFile();
}