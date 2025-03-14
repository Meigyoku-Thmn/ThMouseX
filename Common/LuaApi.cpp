#include "LuaApi.h"
#include "Helper.h"
#include "Helper.Memory.h"
#include "Helper.Encoding.h"
#include "Log.h"
#include "macro.h"
#include "Variables.h"
#include "CallbackStore.h"
#include <MinHook.h>
#include <Windows.h>
#include <string>
#include <format>
#include <span>
#include "PreparationScript.h"

namespace note = common::log;
namespace helper = common::helper;
namespace memory = common::helper::memory;
namespace encoding = common::helper::encoding;
namespace callbackstore = common::callbackstore;

using namespace std;

static DWORD positionAddress;
namespace common::luaapi {
    string LuaJitPrepScript;

    static void Uninitialize(bool isProcessTerminating) {
        SetPositionAddress(NULL);
    }

    void Initialize() {
        callbackstore::RegisterUninitializeCallback(Uninitialize);
        auto dllModule = HINST_THISCOMPONENT;
        auto scriptRes = FindResourceW(dllModule, MAKEINTRESOURCEW(LUAJIT_PREP_SCRIPT), LS_(LUAJIT_PREP_SCRIPT_TYPE));
        if (scriptRes == nil)
            return;
        auto scriptSize = SizeofResource(dllModule, scriptRes);
        auto scriptHandle = LoadResource(dllModule, scriptRes);
        if (scriptHandle == nil)
            return;
        LuaJitPrepScript = string((const char*)LockResource(scriptHandle), scriptSize);
        LuaJitPrepScript
            .append("\n")
            .append(format(SYM_NAME(MH_UNKNOWN)" = {}\n",                   /* */ to_string(MH_UNKNOWN)))
            .append(format(SYM_NAME(MH_UNKNOWN)" = {}\n",                   /* */ to_string(MH_UNKNOWN)))
            .append(format(SYM_NAME(MH_OK)" = {}\n",                        /* */ to_string(MH_OK)))
            .append(format(SYM_NAME(MH_ERROR_ALREADY_INITIALIZED)" = {}\n", /* */ to_string(MH_ERROR_ALREADY_INITIALIZED)))
            .append(format(SYM_NAME(MH_ERROR_NOT_INITIALIZED)" = {}\n",     /* */ to_string(MH_ERROR_NOT_INITIALIZED)))
            .append(format(SYM_NAME(MH_ERROR_ALREADY_CREATED)" = {}\n",     /* */ to_string(MH_ERROR_ALREADY_CREATED)))
            .append(format(SYM_NAME(MH_ERROR_NOT_CREATED)" = {}\n",         /* */ to_string(MH_ERROR_NOT_CREATED)))
            .append(format(SYM_NAME(MH_ERROR_ENABLED)" = {}\n",             /* */ to_string(MH_ERROR_ENABLED)))
            .append(format(SYM_NAME(MH_ERROR_DISABLED)" = {}\n",            /* */ to_string(MH_ERROR_DISABLED)))
            .append(format(SYM_NAME(MH_ERROR_NOT_EXECUTABLE)" = {}\n",      /* */ to_string(MH_ERROR_NOT_EXECUTABLE)))
            .append(format(SYM_NAME(MH_ERROR_UNSUPPORTED_FUNCTION)" = {}\n",/* */ to_string(MH_ERROR_UNSUPPORTED_FUNCTION)))
            .append(format(SYM_NAME(MH_ERROR_MEMORY_ALLOC)" = {}\n",        /* */ to_string(MH_ERROR_MEMORY_ALLOC)))
            .append(format(SYM_NAME(MH_ERROR_MEMORY_PROTECT)" = {}\n",      /* */ to_string(MH_ERROR_MEMORY_PROTECT)))
            .append(format(SYM_NAME(MH_ERROR_MODULE_NOT_FOUND)" = {}\n",    /* */ to_string(MH_ERROR_MODULE_NOT_FOUND)))
            .append(format(SYM_NAME(MH_ERROR_FUNCTION_NOT_FOUND)" = {}\n",  /* */ to_string(MH_ERROR_FUNCTION_NOT_FOUND)))
            ;
    }

    DWORD GetPositionAddress() {
        return positionAddress;
    }

    void SetPositionAddress(DWORD address) {
        positionAddress = address;
    }

    DWORD ReadUInt32(DWORD address) {
        return *PDWORD(address);
    }

    DWORD ResolveAddress(DWORD* offsets, size_t length, bool doNotValidateLastAddress) {
        return memory::ResolveAddress(span{ offsets, length }, doNotValidateLastAddress);
    }

    void OpenConsole() {
        note::OpenConsole();
    }

    PointDataType GetDataType() {
        return g_gameConfig.PosDataType;
    }

    void RegisterUninitializeCallback(UninitializeCallbackType callback) {
        callbackstore::RegisterUninitializeCallback(callback, true);
    }

    MH_STATUS CreateHookApi(LPCSTR pszModule, LPCSTR pszProcName, LPVOID pDetour, LPVOID* ppOriginal, LPCSTR discriminator) {
        return MH_CreateHookApi(encoding::ConvertToUtf16(pszModule).c_str(), pszProcName, pDetour, ppOriginal, discriminator);
    }
}

namespace luaapi = common::luaapi;

DWORD Lua_ReadUInt32(DWORD address) {
    return luaapi::ReadUInt32(address);
}

DWORD Lua_ResolveAddress(DWORD* offsets, size_t length, bool doNotValidateLastAddress) {
    return luaapi::ResolveAddress(offsets, length, doNotValidateLastAddress);
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

int Lua_CreateHook(LPVOID pTarget, LPVOID pDetour, LPVOID* ppOriginal, LPCSTR discriminator) {
    return MH_CreateHook(pTarget, pDetour, ppOriginal, discriminator);
}

int Lua_CreateHookApi(LPCSTR pszModule, LPCSTR pszProcName, LPVOID pDetour, LPVOID* ppOriginal, LPCSTR discriminator) {
    return luaapi::CreateHookApi(pszModule, pszProcName, pDetour, ppOriginal, discriminator);
}

int Lua_EnableHook(LPVOID pTarget, LPCSTR discriminator) {
    return MH_EnableHook(pTarget, discriminator);
}

int Lua_DisableHook(LPVOID pTarget, LPCSTR discriminator) {
    return MH_DisableHook(pTarget, discriminator);
}

LPCSTR Lua_StatusToString(int status) {
    return MH_StatusToString((MH_STATUS)status);
}
