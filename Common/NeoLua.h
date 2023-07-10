#pragma once
#include "framework.h"
#include "macro.h"
#include "DataTypes.h"

DLLEXPORT_C void NeoLua_SetOnClose(DWORD address);
DLLEXPORT_C void NeoLua_SetPositionAddress(DWORD address);
DLLEXPORT_C PointDataType NeoLua_GetDataType();
DLLEXPORT_C void NeoLua_OpenConsole();

namespace common::neolua {
    DWORD GetPositionAddress();
    void Initialize();
    void Uninitialize();
}