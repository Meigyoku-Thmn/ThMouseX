#pragma once
#include "framework.h"
#include "macro.h"

#include "DllLoad.h"

#ifdef COMMON_EXPORTS
DLLEXPORT_C DWORD Common_LuaJIT_ReadUInt32(DWORD address);
DLLEXPORT_C DWORD Common_LuaJIT_ResolveAddress(DWORD* offsets, int length);
DLLEXPORT_C void Common_LuaJIT_OpenConsole();
#endif

namespace common::luajit {
    EXPORT_FUNC(void, LuaJit_, Initialize);
    DWORD GetPositionAddress();
    EXPORT_FUNC(void, LuaJit_, Uninitialize);
}
