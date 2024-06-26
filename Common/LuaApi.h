#pragma once
#include "framework.h"
#include "macro.h"
#include "DataTypes.h"
#include "CallbackStore.h"
#include <MinHook.h>
#include <string>

DLLEXPORT_C MH_STATUS Lua_CreateHookApi(LPCSTR pszModule, LPCSTR pszProcName, LPVOID pDetour, LPVOID *ppOriginal);
DLLEXPORT_C DWORD Lua_ReadUInt32(DWORD address);
DLLEXPORT_C DWORD Lua_ResolveAddress(DWORD* offsets, int length);
DLLEXPORT_C void Lua_OpenConsole();
DLLEXPORT_C void Lua_SetPositionAddress(DWORD address);
DLLEXPORT_C DWORD Lua_GetPositionAddress();
DLLEXPORT_C PointDataType Lua_GetDataType();
DLLEXPORT_C void Lua_RegisterUninitializeCallback(common::callbackstore::UninitializeCallbackType callback);

namespace common::luaapi {
    void Initialize();
    std::string MakePreparationScript();
}