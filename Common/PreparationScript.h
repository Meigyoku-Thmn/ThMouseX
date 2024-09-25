#ifdef COMMON_EXPORTS
#include "macro.h"
#include <Windows.h>
#include "DataTypes.h"
// Include this file only in LuaApi.cpp
#else
-- C Processor-Generated file, do not edit.
-- Lua script expects ThMouseX_ModuleHandle as global variable
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

#ifdef COMMON_EXPORTS
#define LUAFUNC(RetType, FuncName, ...) DLLEXPORT_C RetType Lua_##FuncName(__VA_ARGS__)
#else
#define SYM_NAME(name) #name
#define LUAFUNC(RetType, FuncName, ...) \
    ffi.cdef[[ \
        typedef RetType (*Lua_##FuncName)(__VA_ARGS__);  \
    ]]; \
    FuncName = ffi.cast(SYM_NAME(Lua_##FuncName), ffi.C.GetProcAddress(hModule, SYM_NAME(Lua_##FuncName)))
#endif

LUAFUNC(DWORD,  ReadUInt32, DWORD address);

LUAFUNC(DWORD,  ResolveAddress, DWORD* offsets, size_t length);

LUAFUNC(void,   OpenConsole);

LUAFUNC(void,   SetPositionAddress, DWORD address);

LUAFUNC(int,    GetDataType);

LUAFUNC(void,   RegisterUninitializeCallback, UninitializeCallbackType callback);

LUAFUNC(int,    CreateHook, LPVOID pTarget, LPVOID pDetour, LPVOID *ppOriginal);

LUAFUNC(int,    CreateHookApi, LPCSTR pszModule, LPCSTR pszProcName, LPVOID pDetour, LPVOID *ppOriginal);

LUAFUNC(int,    EnableHook, LPVOID pTarget);

LUAFUNC(int,    RemoveHook, LPVOID pTarget);

LUAFUNC(int,    DisableHook, LPVOID pTarget);

LUAFUNC(LPCSTR, StatusToString, int status);

#ifndef COMMON_EXPORTS
_StatusToString = StatusToString;
function StatusToString(status)
    return ffi.string(_StatusToString(status))
end
#endif