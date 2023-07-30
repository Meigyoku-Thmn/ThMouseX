#pragma once
#include "framework.h"
#include "macro.h"
#include "DataTypes.h"
#include "CallbackStore.h"
#include <string>

DLLEXPORT_C DWORD Lua_ReadUInt32(DWORD address);
DLLEXPORT_C DWORD Lua_ResolveAddress(DWORD* offsets, int length);
DLLEXPORT_C void Lua_OpenConsole();
DLLEXPORT_C void Lua_SetPositionAddress(DWORD address);
DLLEXPORT_C DWORD Lua_GetPositionAddress();
DLLEXPORT_C wchar_t* Lua_ConvertToUtf16Alloc(const char* utf8str);
DLLEXPORT_C void Lua_DeleteUtf16Alloc(wchar_t* utf16);
DLLEXPORT_C PointDataType Lua_GetDataType();
DLLEXPORT_C void Lua_RegisterUninitializeCallback(common::callbackstore::UninitializeCallbackType callback, bool isFromDotNet = false);

namespace common::luaapi {
    void Initialize();
    std::string MakePreparationScriptForLuaJIT();
}