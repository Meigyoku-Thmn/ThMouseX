#pragma once
#include <Windows.h>
#include "macro.h"
#include "DataTypes.h"
#include "CallbackStore.h"
#include <MinHook.h>
#include <string>


#define GET_POSITION_ADDRESS "getPositionAddress"
#define THMOUSEX_MODULE_HANDLE "ThMouseX_ModuleHandle"
#define LUAJIT_PREP_SCRIPT_NAME 101
#define LUAJIT_PREP_SCRIPT_TYPE "LUASCRIPT"

namespace common::luaapi {
    extern std::string LuaJitPrepScript;
    void Initialize();
    DWORD GetPositionAddress();
    void SetPositionAddress(DWORD address);
    DWORD ReadUInt32(DWORD address);
    DWORD ResolveAddress(DWORD* offsets, size_t length);
    void OpenConsole();
    PointDataType GetDataType();
    void RegisterUninitializeCallback(UninitializeCallbackType callback);
    MH_STATUS CreateHookApi(LPCSTR pszModule, LPCSTR pszProcName, LPVOID pDetour, LPVOID *ppOriginal);
}