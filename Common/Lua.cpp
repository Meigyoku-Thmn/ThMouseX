#include "framework.h"
#include <string>
#include <luajit/lua.hpp>
#include <fstream>
#include <sstream>

#include "Lua.h"
#include "LuaApi.h"
#include "Variables.h"
#include "macro.h"
#include "CallbackStore.h"
#include "MinHook.h"
#include "Log.h"
#include "Helper.Encoding.h"

namespace minhook = common::minhook;
namespace callbackstore = common::callbackstore;
namespace note = common::log;
namespace encoding = common::helper::encoding;
namespace luaapi = common::luaapi;

using namespace std;

static bool scriptingDisabled = false;
static bool usePullMechanism = false;

#define GET_POSITION_ADDRESS "getPositionAddress"

static lua_State* L;

#define ImportFunc(lua, luaDllName, funcName) \
(decltype(&funcName))GetProcAddress(lua, #funcName); \
if (!_ ## funcName) { \
    note::ToFile("[Lua] Failed to import %s|" #funcName ".", luaDllName.c_str()); \
    return; \
}0

namespace common::lua {
    int luaL_callmeta_hook(lua_State *L, int obj, const char *e);
    decltype(&luaL_callmeta_hook) ori_luaL_callmeta;
    void lua_call_hook(lua_State *L, int nargs, int nresults);
    decltype(&lua_call_hook) ori_lua_call;
    int lua_cpcall_hook(lua_State *L, lua_CFunction func, void *ud);
    decltype(&lua_cpcall_hook) ori_lua_cpcall;
    int lua_pcall_hook(lua_State *L, int nargs, int nresults, int errfunc);
    decltype(&lua_pcall_hook) ori_lua_pcall;

    decltype(&luaL_callmeta) _luaL_callmeta;
    decltype(&lua_call) _lua_call;
    decltype(&lua_cpcall) _lua_cpcall;
    decltype(&lua_pcall) _lua_pcall;

    decltype(&luaL_loadstring) _luaL_loadstring;
    decltype(&lua_tolstring) _lua_tolstring;
    decltype(&lua_settop) _lua_settop;
    decltype(&lua_tointeger) _lua_tointeger;
    decltype(&lua_isnumber) _lua_isnumber;
    decltype(&lua_pushvalue) _lua_pushvalue;
    decltype(&lua_type) _lua_type;
    decltype(&lua_getfield) _lua_getfield;

    static bool Validate(lua_State* L, int r) {
        if (r != 0) {
            note::ToFile("[Lua] %s", _lua_tolstring(L, -1, nullptr));
            _lua_settop(L, -2);
            scriptingDisabled = true;
            return false;
        }
        return true;
    }

    void AttachScript(lua_State *L);

    string luaDllName;
    string scriptPath;

    void Initialize() {
        if (g_currentConfig.ScriptType != ScriptType::Lua)
            return;

        {
            auto wScriptPath = wstring(g_currentModuleDirPath) + L"/ConfigScripts/" + g_currentConfig.ProcessName + L".lua";
            scriptPath = encoding::ConvertToUtf8(wScriptPath.c_str());
            ifstream scriptFile(scriptPath.c_str());
            if (!scriptFile) {
                note::ToFile("[Lua] Cannot open %s: %s.", scriptPath.c_str(), strerror(errno));
                scriptingDisabled = true;
                return;
            }
            string firstLine;
            if (!getline(scriptFile, firstLine)) {
                note::ToFile("[Lua] Cannot read the first line of %s: %s.", scriptPath.c_str(), strerror(errno));
                scriptingDisabled = true;
                return;
            }
            stringstream lineStream(firstLine);
            string token;
            lineStream >> token;
            if (token != "--") {
                note::ToFile("[Lua] The first line of '%s' is not a Lua comment.", scriptPath.c_str());
                scriptingDisabled = true;
                return;
            }
            lineStream >> token;
            if (token != "LuaDllName") {
                note::ToFile("[Lua] The first Lua comment of '%s' doesn't have the key LuaDllName.", scriptPath.c_str());
                scriptingDisabled = true;
                return;
            }
            lineStream >> token;
            if (token != "=") {
                note::ToFile("[Lua] Expected '=' after LuaDllName in '%s'.", scriptPath.c_str());
                scriptingDisabled = true;
                return;
            }
            token = "";
            lineStream >> token;
            if (token == "") {
                note::ToFile("[Lua] LuaDllName value must be specified in '%s'.", scriptPath.c_str());
                scriptingDisabled = true;
                return;
            }
            luaDllName = token;
        }

        auto luaPath = g_currentProcessDirPath + wstring(L"\\") + encoding::ConvertToUtf16(luaDllName.c_str());
        auto lua = GetModuleHandleW(luaPath.c_str());
        if (!lua) {
            note::ToFile("[Lua] Failed to load %s from the game's directory.", luaDllName.c_str());
            scriptingDisabled = true;
            return;
        }

        _luaL_callmeta = ImportFunc(lua, luaDllName, luaL_callmeta);
        _lua_call = ImportFunc(lua, luaDllName, lua_call);
        _lua_cpcall = ImportFunc(lua, luaDllName, lua_cpcall);
        _lua_pcall = ImportFunc(lua, luaDllName, lua_pcall);

        _luaL_loadstring = ImportFunc(lua, luaDllName, luaL_loadstring);
        _lua_tolstring = ImportFunc(lua, luaDllName, lua_tolstring);
        _lua_settop = ImportFunc(lua, luaDllName, lua_settop);
        _lua_tointeger = ImportFunc(lua, luaDllName, lua_tointeger);
        _lua_isnumber = ImportFunc(lua, luaDllName, lua_isnumber);
        _lua_pushvalue = ImportFunc(lua, luaDllName, lua_pushvalue);
        _lua_type = ImportFunc(lua, luaDllName, lua_type);
        _lua_getfield = ImportFunc(lua, luaDllName, lua_getfield);

        minhook::CreateHook(vector<minhook::HookConfig>{
            {_luaL_callmeta, &luaL_callmeta_hook, (PVOID*)&ori_luaL_callmeta},
            {_lua_call, &lua_call_hook, (PVOID*)&ori_lua_call},
            {_lua_cpcall, &lua_cpcall_hook, (PVOID*)&ori_lua_cpcall},
            {_lua_pcall, &lua_pcall_hook, (PVOID*)&ori_lua_pcall},
        });
    }

    int luaL_callmeta_hook(lua_State *L, int obj, const char *e) {
        auto rs = ori_luaL_callmeta(L, obj, e);
        AttachScript(L);
        minhook::RemoveHooks(vector<minhook::HookConfig> { { _luaL_callmeta, nullptr , (PVOID*)ori_luaL_callmeta } });
        return rs;
    }
    void lua_call_hook(lua_State *L, int nargs, int nresults) {
        ori_lua_call(L, nargs, nresults);
        AttachScript(L);
        minhook::RemoveHooks(vector<minhook::HookConfig> { { _lua_call, nullptr, (PVOID*)ori_lua_call } });
        return;
    }
    int lua_cpcall_hook(lua_State *L, lua_CFunction func, void *ud) {
        auto rs = ori_lua_cpcall(L, func, ud);
        AttachScript(L);
        minhook::RemoveHooks(vector<minhook::HookConfig> { { _lua_cpcall, nullptr, (PVOID*)ori_lua_cpcall } });
        return rs;
    }
    int lua_pcall_hook(lua_State *L, int nargs, int nresults, int errfunc) {
        auto rs = ori_lua_pcall(L, nargs, nresults, errfunc);
        AttachScript(L);
        minhook::RemoveHooks(vector<minhook::HookConfig> { { _lua_pcall, nullptr, (PVOID*)ori_lua_pcall } });
        return rs;
    }

    void AttachScript(lua_State *L) {
        static bool scriptAttached = false;
        if (scriptAttached)
            return;
        scriptAttached = true;

        ::L = L;

        auto rs = 0;
        if ((rs = _luaL_loadstring(L, luaapi::MakePreparationScript().c_str())) == 0)
            rs = ori_lua_pcall(L, 0, LUA_MULTRET, 0);
        if (!Validate(L, rs))
            return;

        auto scriptIn = fopen(scriptPath.c_str(), "rb");
        if (scriptIn == nullptr) {
            note::ToFile("[Lua] Cannot open %s: %s.", scriptPath.c_str(), strerror(errno));
            scriptingDisabled = true;
            return;
        }
        fseek(scriptIn, 0, SEEK_END);
        auto scriptSize = ftell(scriptIn);
        auto scriptContent = vector<char>(scriptSize + 1);
        rewind(scriptIn);
        fread(scriptContent.data(), sizeof(scriptContent[0]), scriptSize + 1, scriptIn);
        scriptContent[scriptSize] = '\0';
        fclose(scriptIn);
        if ((rs = _luaL_loadstring(L, scriptContent.data())) == 0)
            rs = ori_lua_pcall(L, 0, LUA_MULTRET, 0);
        if (!Validate(L, rs))
            return;

        _lua_getfield(L, LUA_GLOBALSINDEX, GET_POSITION_ADDRESS);
        if (_lua_type(L, -1) == LUA_TFUNCTION)
            usePullMechanism = true;
        _lua_settop(L, -2);
    }

    DWORD GetPositionAddress() {
        if (scriptingDisabled)
            return NULL;

        if (!usePullMechanism)
            return Lua_GetPositionAddress();

        _lua_getfield(L, LUA_GLOBALSINDEX, GET_POSITION_ADDRESS);
        if (!Validate(L, _lua_pcall(L, 0, 1, 0)))
            return NULL;

        if (!_lua_isnumber(L, -1)) {
            note::ToFile("[Lua] The value returned from " GET_POSITION_ADDRESS " wasn't a number.");
            scriptingDisabled = true;
            return NULL;
        }

        auto result = (DWORD)_lua_tointeger(L, -1);
        _lua_settop(L, -2);
        return result;
    }
}
