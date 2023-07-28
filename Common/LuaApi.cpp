#include "LuaApi.h"
#include "Helper.h"
#include "Helper.Memory.h"
#include "Helper.Encoding.h"
#include "Log.h"
#include "Variables.h"
#include "CallbackStore.h"
#include <string>
#include <format>

namespace note = common::log;
namespace helper = common::helper;
namespace memory = common::helper::memory;
namespace encoding = common::helper::encoding;
namespace callbackstore = common::callbackstore;

using namespace std;

DWORD Lua_ReadUInt32(DWORD address) {
    return *PDWORD(address);
}

DWORD Lua_ResolveAddress(DWORD* offsets, int length) {
    return memory::ResolveAddress(offsets, length);
}

void Lua_OpenConsole() {
    note::OpenConsole();
}

static DWORD positionAddress;

void Lua_SetPositionAddress(DWORD address) {
    positionAddress = address;
}

DWORD Lua_GetPositionAddress() {
    return positionAddress;
}

wchar_t* Lua_ConvertToUtf16Alloc(const char* utf8str) {
    auto utf16str = encoding::ConvertToUtf16(utf8str);
    auto utf16raw = new wchar_t[utf16str.size() + 1];
    return wcscpy(utf16raw, utf16str.c_str());
}

void Lua_DeleteUtf16Alloc(wchar_t* utf16) {
    delete[] utf16;
}

PointDataType Lua_GetDataType() {
    return g_currentConfig.PosDataType;
}

void Lua_RegisterUninitializeCallback(common::callbackstore::UninitializeCallbackType callback) {
    callbackstore::RegisterUninitializeCallback(callback);
}

string MakePreparationScriptForLuaJIT() {
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