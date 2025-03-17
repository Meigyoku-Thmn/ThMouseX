#pragma once
#include <Windows.h>
#include "macro.h"
#include "DataTypes.h"
#include "CallbackStore.h"
#include <MinHook.h>
#include <string>
#include <cstdint>

#include "../ThMouseX/resource.h"

#define GET_POSITION_ADDRESS "getPositionAddress"
#define THMOUSEX_MODULE_HANDLE "ThMouseX_ModuleHandle"

namespace common::luaapi {
    extern std::string LuaJitPrepScript;
    void Initialize();
    uintptr_t GetPositionAddress();
    void SetPositionAddress(uintptr_t address);
    DWORD ReadUInt32(DWORD address);
    uintptr_t ResolveAddress(DWORD* offsets, size_t length, bool doNotValidateLastAddress);
    void OpenConsole();
    PointDataType GetDataType();
    void RegisterUninitializeCallback(UninitializeCallbackType callback);
    MH_STATUS CreateHookApi(LPCSTR pszModule, LPCSTR pszProcName, LPVOID pDetour, LPVOID *ppOriginal, LPCSTR discriminator);
}