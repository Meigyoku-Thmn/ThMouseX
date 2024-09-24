#pragma once

#ifdef WIN32
#include <Windows.h>
#include "DataTypes.h"
#include "macro.h"
// Lua script expects ThMouseX_ModuleHandle as global variable
#else
local ffi = require("ffi")
ffi.cdef [[
    typedef unsigned long DWORD;
    typedef void* LPVOID;
    typedef LPVOID HMODULE;
    typedef const wchar_t* LPCWSTR;
    typedef const char* LPCSTR;
    typedef void(__cdecl *UninitializeCallbackType)(bool isProcessTerminating);
    typedef void(__cdecl *CallbackType)();
    LPVOID GetProcAddress(HMODULE hModule, LPCSTR lpProcName);
]]
    local hModule = ffi.cast("HMODULE", ThMouseX_ModuleHandle)
#endif

#ifdef WIN32
#define LUAFUNC(RetType, FuncName, ...) DLLEXPORT_C RetType FuncName(__VA_ARGS__)
#else
#define LUAFUNC(RetType, FuncName, ...) \
    ffi.cdef[[ \
        typedef RetType (*FuncName)(__VA_ARGS__);  \
    ]]; \
    _G[#FuncName] = ffi.cast(#FuncName, ffi.C.GetProcAddress(hModule, #FuncName))
#endif

LUAFUNC(DWORD,  Lua_ReadUInt32, DWORD address);
LUAFUNC(DWORD,  Lua_ResolveAddress, DWORD* offsets, size_t length);
LUAFUNC(void,   Lua_OpenConsole);
LUAFUNC(void,   Lua_SetPositionAddress, DWORD address);
LUAFUNC(int,    Lua_GetDataType);
LUAFUNC(void,   Lua_RegisterUninitializeCallback, UninitializeCallbackType callback);
LUAFUNC(int,    Lua_CreateHook, LPVOID pTarget, LPVOID pDetour, LPVOID *ppOriginal);
LUAFUNC(int,    Lua_CreateHookApi, LPCSTR pszModule, LPCSTR pszProcName, LPVOID pDetour, LPVOID *ppOriginal);
LUAFUNC(int,    Lua_EnableHook, LPVOID pTarget);
LUAFUNC(int,    Lua_RemoveHook, LPVOID pTarget);
LUAFUNC(int,    Lua_DisableHook, LPVOID pTarget);
LUAFUNC(LPCSTR, Lua_StatusToString, int status);

#ifndef WIN32
_Lua_StatusToString = Lua_StatusToString;
function Lua_StatusToString(status)
    return ffi.string(_Lua_StatusToString(status))
end
#endif