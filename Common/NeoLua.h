#pragma once
#include "framework.h"
#include "macro.h"
#include "DataTypes.h"

#include "DllLoad.h"

#ifdef COMMON_EXPORTS
DLLEXPORT_C void Common_NeoLua_SetOnClose(DWORD address);
DLLEXPORT_C void Common_NeoLua_SetPositionAddress(DWORD address);
DLLEXPORT_C PointDataType Common_NeoLua_GetDataType();
DLLEXPORT_C void Common_NeoLua_OpenConsole();
#endif

namespace common::neolua {
    DWORD GetPositionAddress();
    EXPORT_FUNC(void, NeoLua_, Initialize);
    EXPORT_FUNC(void, NeoLua_, Uninitialize);
}