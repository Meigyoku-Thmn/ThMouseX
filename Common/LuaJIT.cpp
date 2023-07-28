#include "framework.h"
#include <string>
#include <codecvt>
#include <luajit/lua.hpp>
#include <format>

#include "MinHook.h"
#include "macro.h"
#include "CallbackStore.h"
#include "LuaJIT.h"
#include "Log.h"
#include "Helper.h"
#include "Helper.Memory.h"
#include "Helper.Encoding.h"
#include "Variables.h"

namespace note = common::log;
namespace helper = common::helper;
namespace memory = common::helper::memory;
namespace encoding = common::helper::encoding;
namespace minhook = common::minhook;
namespace callbackstore = common::callbackstore;

using namespace std;

DLLEXPORT_C DWORD LuaJIT_ReadUInt32(DWORD address) {
    return *PDWORD(address);
}

DLLEXPORT_C DWORD LuaJIT_ResolveAddress(DWORD* offsets, int length) {
    return memory::ResolveAddress(offsets, length);
}

DLLEXPORT_C void LuaJIT_OpenConsole() {
    note::OpenConsole();
}

static DWORD positionAddress;
DLLEXPORT_C void LuaJIT_SetPositionAddress(DWORD address) {
    positionAddress = address;
}


DLLEXPORT_C wchar_t* LuaJIT_ConvertToUtf16Alloc(const char* utf8str) {
    auto utf16str = encoding::ConvertToUtf16(utf8str);
    auto utf16raw = new wchar_t[utf16str.size() + 1];
    return wcscpy(utf16raw, utf16str.c_str());
}

DLLEXPORT_C void LuaJIT_DeleteUtf16Alloc(wchar_t* utf16) {
    delete[] utf16;
}

string GetPreparationScript() {
    auto thisDllPath = encoding::ConvertToUtf8((wstring(g_currentModuleDirPath) + L"\\" + L_(APP_NAME)).c_str());
    helper::Replace(thisDllPath, "\\", "\\\\");
    auto preparationScript = format(R"(
        local ffi = require("ffi")

        ffi.cdef [[
            uint32_t LuaJIT_ReadUInt32          (uint32_t address);
            uint32_t LuaJIT_ResolveAddress      (uint32_t* offsets, int length);
            void     LuaJIT_OpenConsole         ();
            void     LuaJIT_SetPositionAddress  (uint32_t address);

            int32_t stdcall MH_CreateHook      (uint32_t* pTarget, uint32_t* pDetour, uint32_t** ppOriginal);
            int32_t stdcall MH_CreateHookApi   (wchar_t* pszModule, char* pszProcName, uint32_t* pDetour, uint32_t** ppOriginal);
            int32_t stdcall MH_EnableHook      (uint32_t* pTarget);
            int32_t stdcall MH_RemoveHook      (uint32_t* pTarget);
            int32_t stdcall MH_DisableHook     (uint32_t* pTarget);
            char*   stdcall MH_StatusToString  (int32_t status);

            wchar_t* LuaJIT_ConvertToUtf16Alloc(const char* utf8str);
            void     LuaJIT_DeleteUtf16Alloc(wchar_t* utf16)
        ]]

        local ThMouseX = ffi.load("{}")

        function ReadUInt32(address)
            return ThMouseX.LuaJIT_ReadUInt32(address)
        end

        function ResolveAddress(addressChain, length)
            return ThMouseX.LuaJIT_ResolveAddress(addressChain, length)
        end

        function OpenConsole()
            return ThMouseX.LuaJIT_OpenConsole()
        end

        function SetPositionAddress(address)
            return ThMouseX.LuaJIT_SetPositionAddress(address)
        end

        function CreateHook(target, detour, outOriginal)
            return MH_CreateHook(target, detour, outOriginal)
        end

        function CreateHookApi(module, procName, detour, outOriginal)
            local moduleUtf16 = ThMouseX.LuaJIT_ConvertToUtf16Alloc(module)
            local rs = MH_CreateHookApi(moduleUtf16, procName, detour, outOriginal)
            LuaJIT_DeleteUtf16Alloc(moduleUtf16)
            return rs;
        end

        function RemoveHook(target)
            return MH_RemoveHook(target)
        end

        function HookStatusToString(status)
            return ffi.string(MH_StatusToString(status))
        end
    )", thisDllPath);
    return preparationScript;
}

bool scriptingDisabled = false;

#define GET_POSITION_ADDRESS "getPositionAddress"

lua_State* L;

bool CheckAndDisableIfError(lua_State* L, int r) {
    if (r != 0) {
        note::ToFile("[LuaJIT] %s", lua_tostring(L, -1));
        scriptingDisabled = true;
        return false;
    }
    return true;
}

namespace common::luajit {
    HMODULE WINAPI _LoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
    decltype(&_LoadLibraryExA) OriLoadLibraryExA;

    HMODULE WINAPI _LoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
        auto thisDllPath = wstring(g_currentModuleDirPath) + L"\\" + L_(APP_NAME);
        if (strcmp(lpLibFileName, encoding::ConvertToUtf8(thisDllPath.c_str()).c_str()) == 0)
            return LoadLibraryExW(thisDllPath.c_str(), hFile, dwFlags);
        else
            return OriLoadLibraryExA(lpLibFileName, hFile, dwFlags);
    }

    void Uninitialize(bool isProcessTerminating) {
        if (L != NULL)
            lua_close(L);
    }

    void Initialize() {
        if (g_currentConfig.ScriptType != ScriptType::LuaJIT)
            return;
        // Only support Detached
        if (g_currentConfig.ScriptRunPlace != ScriptRunPlace::Detached)
            return;
        // Can support Pull and Push
        if (g_currentConfig.ScriptPositionGetMethod == ScriptPositionGetMethod::None)
            return;

        L = luaL_newstate();
        if (!L) {
            note::ToFile("[LuaJIT] %s", "Failed to initialize LuaJIT.");
            scriptingDisabled = true;
            return;
        }

        callbackstore::RegisterUninitializeCallback(Uninitialize);

        luaL_openlibs(L);

        vector<minhook::HookApiConfig> hookConfig{
            {L"KERNEL32.DLL", "LoadLibraryExA", &_LoadLibraryExA, (PVOID*)&OriLoadLibraryExA},
        };

        if (!minhook::CreateApiHook(hookConfig)) {
            note::ToFile("[LuaJIT] Failed to hook LoadLibraryExA (CreateApiHook).");
            return;
        }

        if (!minhook::EnableHooks(hookConfig)) {
            note::ToFile("[LuaJIT] Failed to hook LoadLibraryExA (EnableHooks).");
            return;
        }

        if (!CheckAndDisableIfError(L, luaL_dostring(L, GetPreparationScript().c_str()))) {
            minhook::DisableHooks(hookConfig);
            note::ToFile("[LuaJIT] The above error occurred in PreparationScript.");
            return;
        }

        minhook::DisableHooks(hookConfig);

        auto wScriptPath = wstring(g_currentModuleDirPath) + L"/ConfigScripts/" + g_currentConfig.ProcessName + L".lua";
        auto scriptPath = encoding::ConvertToUtf8(wScriptPath.c_str());

        if (!CheckAndDisableIfError(L, luaL_dofile(L, scriptPath.c_str())))
            return;

        if (g_currentConfig.ScriptPositionGetMethod == ScriptPositionGetMethod::Push)
            return;

        lua_getglobal(L, GET_POSITION_ADDRESS);
        if (!lua_isfunction(L, -1)) {
            note::ToFile("[LuaJIT] %s", GET_POSITION_ADDRESS " function not found in global scope.");
            scriptingDisabled = true;
            return;
        }
    }

    DWORD GetPositionAddress() {
        if (scriptingDisabled)
            return NULL;

        if (g_currentConfig.ScriptPositionGetMethod == ScriptPositionGetMethod::Push) {
            return positionAddress;
        }

        lua_pushvalue(L, -1);

        if (!CheckAndDisableIfError(L, lua_pcall(L, 0, 1, 0)))
            return NULL;

        if (!lua_isnumber(L, -1)) {
            note::ToFile("[LuaJIT] %s", "The value returned from " GET_POSITION_ADDRESS " wasn't a number.");
            scriptingDisabled = true;
            return NULL;
        }

        auto result = (DWORD)lua_tointeger(L, -1);
        lua_pop(L, 1);
        return result;
    }
}
