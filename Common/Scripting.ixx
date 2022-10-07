module;

#include "framework.h"
#include <string>
#include <codecvt>
#include "macro.h"
#include "luajit/lua.hpp"

export module common.scripting;

import common.var;
import common.log;

using namespace std;

export bool scriptingDisabled = false;

#define GET_POSITION_ADDRESS "getPositionAddress"

lua_State* L;

bool CheckAndDisableIfError(lua_State *L, int r) {
    if (r != 0) {
        FileLog("[LuaJIT] %s\n", lua_tostring(L, -1));
        scriptingDisabled = true;
        return false;
    }
    return true;
}

export DLLEXPORT void InitializeScripting() {
    if (!g_currentConfig.CalcAddressByScripting)
        return;

    L = luaL_newstate();
    if (L == NULL) {
        FileLog("[LuaJIT] %s\n", "Failed to initialize LuaJIT.");
        scriptingDisabled = true;
        return;
    }

    luaL_openlibs(L);

    auto scriptPath = wstring_convert<codecvt_utf8_utf16<wchar_t>>().to_bytes(
        wstring(g_currentModuleDirPath) + L"/ConfigScripts/" + g_currentConfig.ProcessName + L".lua");

    if (!CheckAndDisableIfError(L, luaL_dofile(L, scriptPath.c_str())))
        return;

    lua_getglobal(L, GET_POSITION_ADDRESS);
    if (!lua_isfunction(L, -1)) {
        FileLog("[LuaJIT] %s\n", GET_POSITION_ADDRESS " function not found in global scope.");
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
        FileLog("[LuaJIT] %s\n", "The value returned from " GET_POSITION_ADDRESS " wasn't a number.");
        scriptingDisabled = true;
        return NULL;
    }

    auto result = (DWORD)lua_tointeger(L, -1);
    lua_pop(L, 1);
    return result;
}

export DLLEXPORT void UninitializeScripting() {
    if (L != NULL)
        lua_close(L);
}