#pragma once
#include "macro.h"

namespace core::configurate {
    DLLEXPORT_C bool PopulateMethodRVAs();
    DLLEXPORT_C bool ReadGamesFile();
    DLLEXPORT_C bool ReadGeneralConfigFile();
}