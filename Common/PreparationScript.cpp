#include "PreparationScript.h"
#include <Windows.h>
#include "DataTypes.h"
#include "macro.h"
#include "LuaApi.h"
#include <MinHook.h>

namespace luaapi = common::luaapi;

DWORD Lua_ReadUInt32(DWORD address) {
    return luaapi::ReadUInt32(address);
}

DWORD Lua_ResolveAddress(DWORD* offsets, size_t length) {
    return luaapi::ResolveAddress(offsets, length);
}

void Lua_OpenConsole() {
    luaapi::OpenConsole();
}

void Lua_SetPositionAddress(DWORD address) {
    luaapi::SetPositionAddress(address);
}

int Lua_GetDataType() {
    return luaapi::GetDataType();
}

void Lua_RegisterUninitializeCallback(UninitializeCallbackType callback) {
    luaapi::RegisterUninitializeCallback(callback);
}

int Lua_CreateHook(LPVOID pTarget, LPVOID pDetour, LPVOID* ppOriginal) {
    return MH_CreateHook(pTarget, pDetour, ppOriginal);
}

int Lua_CreateHookApi(LPCSTR pszModule, LPCSTR pszProcName, LPVOID pDetour, LPVOID* ppOriginal) {
    return luaapi::CreateHookApi(pszModule, pszProcName, pDetour, ppOriginal);
}

int Lua_EnableHook(LPVOID pTarget) {
    return MH_EnableHook(pTarget);
}

int Lua_RemoveHook(LPVOID pTarget) {
    return MH_RemoveHook(pTarget);
}

int Lua_DisableHook(LPVOID pTarget) {
    return MH_DisableHook(pTarget);
}

LPCSTR Lua_StatusToString(int status) {
    return MH_StatusToString((MH_STATUS)status);
}
