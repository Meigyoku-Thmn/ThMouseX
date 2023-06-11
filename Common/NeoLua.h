#pragma once
#include "framework.h"
#include "macro.h"
#include "DataTypes.h"

DLLEXPORT_C void Common_NeoLua_SetOnClose(DWORD address);
DLLEXPORT_C void Common_NeoLua_SetPositionAddress(DWORD address);
DLLEXPORT_C PointDataType Common_NeoLua_GetDataType();
DLLEXPORT_C void Common_NeoLua_OpenConsole();

namespace common::neolua {
    DWORD GetPositionAddress();
    DLLEXPORT void Initialize();
    DLLEXPORT void Uninitialize();
}
