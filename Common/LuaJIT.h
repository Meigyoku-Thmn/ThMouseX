#pragma once
#include "framework.h"
#include "macro.h"

DLLEXPORT_C DWORD Common_LuaJIT_ReadUInt32(DWORD address);
DLLEXPORT_C DWORD Common_LuaJIT_ResolveAddress(DWORD* offsets, int length);
DLLEXPORT_C void Common_LuaJIT_OpenConsole();

namespace common::luajit {
    DLLEXPORT void Initialize();
    DWORD GetPositionAddress();
    DLLEXPORT void Uninitialize();
}
