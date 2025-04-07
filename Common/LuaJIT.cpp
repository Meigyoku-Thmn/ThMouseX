#include <Windows.h>
#include <string>
#include <codecvt>
#include <cstdint>
#include <format>
#include <luajit/lua.hpp>

#include "macro.h"
#include "CallbackStore.h"
#include "LuaJIT.h"
#include "Log.h"
#include "Helper.h"
#include "Helper.Memory.h"
#include "Helper.Encoding.h"
#include "Variables.h"
#include "LuaApi.h"

namespace note = common::log;
namespace helper = common::helper;
namespace memory = common::helper::memory;
namespace encoding = common::helper::encoding;
namespace callbackstore = common::callbackstore;
namespace luaapi = common::luaapi;

using namespace std;

static bool scriptingDisabled = false;
static bool usePullMechanism = false;

static lua_State* L;

static bool CheckAndDisableIfError(lua_State* _L, int r) {
    if (r != LUA_OK) {
        note::ToFile("[LuaJIT] %s", lua_tostring(_L, -1));
        scriptingDisabled = true;
        return false;
    }
    return true;
}

namespace common::luajit {
    static void Uninitialize(bool isProcessTerminating) {
        if (!isProcessTerminating && L != nil)
            lua_close(L);
    }

    void Initialize() {
        using enum ScriptType;
        if (g_gameConfig.ScriptType != LuaJIT)
            return;

        L = luaL_newstate();
        if (!L) {
            note::ToFile("[LuaJIT] %s", "Failed to initialize LuaJIT.");
            scriptingDisabled = true;
            return;
        }

        callbackstore::RegisterUninitializeCallback(Uninitialize);

        luaL_openlibs(L);

        auto oldStackSize = lua_gettop(L);

        lua_pushinteger(L, bcast<uintptr_t>(g_coreModule));
        lua_setglobal(L, THMOUSEX_MODULE_HANDLE);
        if (!CheckAndDisableIfError(L, luaL_dostring(L, luaapi::LuaJitPrepScript.c_str()))) {
            note::ToFile("[LuaJIT] The above error occurred in Preparation Script.");
            lua_settop(L, oldStackSize);
            return;
        }

        auto wScriptPath = format(L"{}/ConfigScripts/{}.lua", g_currentModuleDirPath, g_gameConfig.processName);
        auto scriptPath = encoding::ConvertToUtf8(wScriptPath);

        if (!CheckAndDisableIfError(L, luaL_dofile(L, scriptPath.c_str()))) {
            lua_settop(L, oldStackSize);
            return;
        }

        lua_getglobal(L, GET_POSITION_ADDRESS);
        if (lua_isfunction(L, -1))
            usePullMechanism = true;
        else
            lua_settop(L, oldStackSize);
    }

    uintptr_t GetPositionAddress() {
        if (scriptingDisabled)
            return NULL;

        if (!usePullMechanism)
            return luaapi::GetPositionAddress();

        auto oldStackSize = lua_gettop(L);

        lua_pushvalue(L, -1);

        if (!CheckAndDisableIfError(L, lua_pcall(L, 0, 1, 0))) {
            lua_settop(L, oldStackSize);
            return NULL;
        }

        if (!lua_isnumber(L, -1)) {
            note::ToFile("[LuaJIT] %s", "The value returned from " GET_POSITION_ADDRESS " wasn't a number.");
            scriptingDisabled = true;
            lua_settop(L, oldStackSize);
            return NULL;
        }

        auto result = scast<uintptr_t>(lua_tointeger(L, -1));
        lua_settop(L, oldStackSize);
        return result;
    }
}
