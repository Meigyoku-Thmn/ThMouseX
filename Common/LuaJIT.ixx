module;

#include "framework.h"
#include <string>
#include <codecvt>
#include "macro.h"
#include "luajit/lua.hpp"

export module common.luajit;

import common.var;
import common.log;
import common.helper.memory;

namespace note = common::log;
namespace memory = common::helper::memory;

using namespace std;

DLLEXPORT_C DWORD Common_LuaJIT_ReadUInt32(DWORD address) {
    return *PDWORD(address);
}

DLLEXPORT_C DWORD Common_LuaJIT_ResolveAddress(DWORD* offsets, int length) {
    return memory::ResolveAddress(offsets, length);
}

DLLEXPORT_C void Common_LuaJIT_OpenConsole() {
    note::OpenConsole();
}

auto PreparationScript = R"(
    local ffi = require("ffi")

    ffi.cdef [[
        uint32_t Common_LuaJIT_ReadUInt32     (uint32_t address);
        uint32_t Common_LuaJIT_ResolveAddress (uint32_t* offsets, int length);
        void     Common_LuaJIT_OpenConsole ();
    ]]

    local ThMouseX = ffi.load('Common.dll')

    function OpenConsole()
        return ThMouseX.Common_LuaJIT_OpenConsole()
    end

    function ReadUInt32(address)
        return ThMouseX.Common_LuaJIT_ReadUInt32(address)
    end

    function ResolveAddress(addressChain, length)
        return ThMouseX.Common_LuaJIT_ResolveAddress(addressChain, length)
    end

    function AllocNew(...)
        return ffi.new(unpack({...}))
    end
)";

bool scriptingDisabled = false;

#define GET_POSITION_ADDRESS "getPositionAddress"

lua_State* L;

bool CheckAndDisableIfError(lua_State *L, int r) {
    if (r != 0) {
        note::ToFile("[LuaJIT] %s", lua_tostring(L, -1));
        scriptingDisabled = true;
        return false;
    }
    return true;
}

namespace common::luajit {
    export DLLEXPORT void Initialize() {
        if (g_currentConfig.ScriptingMethodToFindAddress != ScriptingMethod::LuaJIT)
            return;

        L = luaL_newstate();
        if (L == NULL) {
            note::ToFile("[LuaJIT] %s", "Failed to initialize LuaJIT.");
            scriptingDisabled = true;
            return;
        }

        luaL_openlibs(L);

        if (!CheckAndDisableIfError(L, luaL_dostring(L, PreparationScript))) {
            note::ToFile("[LuaJIT] The above error occurred in PreparationScript.");
            return;
        }

        auto scriptPath = wstring_convert<codecvt_utf8_utf16<wchar_t>>().to_bytes(
            wstring(g_currentModuleDirPath) + L"/ConfigScripts/" + g_currentConfig.ProcessName + L".lua");

        if (!CheckAndDisableIfError(L, luaL_dofile(L, scriptPath.c_str())))
            return;

        lua_getglobal(L, GET_POSITION_ADDRESS);
        if (!lua_isfunction(L, -1)) {
            note::ToFile("[LuaJIT] %s", GET_POSITION_ADDRESS " function not found in global scope.");
            scriptingDisabled = true;
            return;
        }
    }

    export DWORD GetPositionAddress() {
        if (scriptingDisabled)
            return NULL;

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

    export DLLEXPORT void Uninitialize() {
        if (L != NULL)
            lua_close(L);
    }
}
