#include "framework.h"
#include <string>
#include <codecvt>
#include "macro.h"
#include "luajit/lua.hpp"

#include "LuaJIT.h"
#include "Log.h"
#include "Helper.Memory.h"
#include "Helper.Encoding.h"
#include "Variables.h"

namespace note = common::log;
namespace memory = common::helper::memory;
namespace encoding = common::helper::encoding;

using namespace std;

DWORD Common_LuaJIT_ReadUInt32(DWORD address) {
    return *PDWORD(address);
}

DWORD Common_LuaJIT_ResolveAddress(DWORD* offsets, int length) {
    return memory::ResolveAddress(offsets, length);
}

void Common_LuaJIT_OpenConsole() {
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
    void Initialize() {
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

        auto wScriptPath = wstring(g_currentModuleDirPath) + L"/ConfigScripts/" + g_currentConfig.ProcessName + L".lua";
        auto scriptPath = encoding::ConvertToUtf8(wScriptPath.c_str());

        if (!CheckAndDisableIfError(L, luaL_dofile(L, scriptPath.c_str())))
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

    void Uninitialize() {
        if (L != NULL)
            lua_close(L);
    }
}
