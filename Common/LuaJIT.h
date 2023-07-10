#pragma once
#include "framework.h"
#include "macro.h"

DLLEXPORT_C DWORD LuaJIT_ReadUInt32(DWORD address);
DLLEXPORT_C DWORD LuaJIT_ResolveAddress(DWORD* offsets, int length);
DLLEXPORT_C void LuaJIT_OpenConsole();

namespace common::luajit {
    void Initialize();
    DWORD GetPositionAddress();
}
