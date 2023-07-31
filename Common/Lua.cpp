#include "framework.h"
#include <string>
#include <luajit/lua.hpp>
#include <fstream>

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

using namespace std;

namespace common::lua {
    int luaL_callmeta_hook(lua_State *L, int obj, const char *e);
    decltype(&luaL_callmeta_hook) ori_luaL_callmeta;
    void lua_call_hook(lua_State *L, int nargs, int nresults);
    decltype(&lua_call_hook) ori_lua_call;
    int lua_cpcall_hook(lua_State *L, lua_CFunction func, void *ud);
    decltype(&lua_cpcall_hook) ori_lua_cpcall;
    int lua_pcall_hook(lua_State *L, int nargs, int nresults, int errfunc);
    decltype(&lua_pcall_hook) ori_lua_pcall;

    decltype(&luaL_loadstring) _luaL_loadstring;
    decltype(&lua_tolstring) _lua_tolstring;
    decltype(&lua_settop) _lua_settop;

    bool Validate(lua_State* L, int r) {
        if (r != 0) {
            note::ToFile("[Lua] %s", _lua_tolstring(L, -1, 0));
            _lua_settop(L, -2);
            return false;
        }
        return true;
    }

    void AttachScript(lua_State *L);

    wstring luaDllName;
    string luaDllNameUtf8;

    void Initialize() {
        if (g_currentConfig.ScriptType != ScriptType::Lua)
            return;
        // Only support Attached
        if (g_currentConfig.ScriptRunPlace != ScriptRunPlace::Attached)
            return;
        // Only support Push
        if (g_currentConfig.ScriptPositionGetMethod != ScriptPositionGetMethod::Push)
            return;

        luaDllName = L"lua.dll";
        luaDllNameUtf8 = "lua.dll";

        auto lua = GetModuleHandleW((g_currentProcessDirPath + wstring(L"\\") + luaDllName).c_str());
        if (!lua) {
            note::ToFile("[Lua] Failed to load %s from the game's directory.", luaDllNameUtf8.c_str());
            return;
        }

        _luaL_loadstring = (decltype(&luaL_loadstring))GetProcAddress(lua, "luaL_loadstring");
        if (!_luaL_loadstring) {
            note::ToFile("[Lua] Failed to import %s|luaL_loadstring.", luaDllNameUtf8.c_str());
            return;
        }
        _lua_tolstring = (decltype(&lua_tolstring))GetProcAddress(lua, "lua_tolstring");
        if (!_lua_tolstring) {
            note::ToFile("[Lua] Failed to import %s|lua_tolstring.", luaDllNameUtf8.c_str());
            return;
        }
        _lua_settop = (decltype(&lua_settop))GetProcAddress(lua, "lua_settop");
        if (!_lua_settop) {
            note::ToFile("[Lua] Failed to import %s|lua_settop.", luaDllNameUtf8.c_str());
            return;
        }

        minhook::CreateApiHook(vector<minhook::HookApiConfig> {
            { luaDllName.c_str(), "luaL_callmeta", &luaL_callmeta_hook, (PVOID*)&ori_luaL_callmeta },
            { luaDllName.c_str(), "lua_call", &lua_call_hook, (PVOID*)&ori_lua_call },
            { luaDllName.c_str(), "lua_cpcall", &lua_cpcall_hook, (PVOID*)&ori_lua_cpcall },
            { luaDllName.c_str(), "lua_pcall", &lua_pcall_hook, (PVOID*)&ori_lua_pcall },
        });
    }

    int luaL_callmeta_hook(lua_State *L, int obj, const char *e) {
        auto rs = ori_luaL_callmeta(L, obj, e);
        AttachScript(L);
        minhook::RemoveHooks(vector<minhook::HookApiConfig> {
            { luaDllName.c_str(), "luaL_callmeta", NULL, NULL }
        });
        return rs;
    }
    void lua_call_hook(lua_State *L, int nargs, int nresults) {
        ori_lua_call(L, nargs, nresults);
        AttachScript(L);
        minhook::RemoveHooks(vector<minhook::HookApiConfig> {
            { luaDllName.c_str(), "lua_call", NULL, NULL }
        });
        return;
    }
    int lua_cpcall_hook(lua_State *L, lua_CFunction func, void *ud) {
        auto rs = ori_lua_cpcall(L, func, ud);
        AttachScript(L);
        minhook::RemoveHooks(vector<minhook::HookApiConfig> {
            { luaDllName.c_str(), "lua_cpcall", NULL, NULL }
        });
        return rs;
    }
    int lua_pcall_hook(lua_State *L, int nargs, int nresults, int errfunc) {
        auto rs = ori_lua_pcall(L, nargs, nresults, errfunc);
        AttachScript(L);
        minhook::RemoveHooks(vector<minhook::HookApiConfig> {
            { luaDllName.c_str(), "lua_pcall", NULL, NULL }
        });
        return rs;
    }

    void AttachScript(lua_State *L) {
        static bool scriptAttached = false;
        if (scriptAttached)
            return;
        scriptAttached = true;

        auto wScriptPath = wstring(g_currentModuleDirPath) + L"/ConfigScripts/" + g_currentConfig.ProcessName + L".lua";
        auto scriptPath = encoding::ConvertToUtf8(wScriptPath.c_str());
        auto scriptIn = fopen(scriptPath.c_str(), "rb");
        if (scriptIn == NULL) {
            note::ToFile("[Lua] Cannot open %s: %s", scriptPath.c_str(), strerror(errno));
            return;
        }
        fseek(scriptIn, 0, SEEK_END);
        auto scriptSize = ftell(scriptIn);
        auto scriptContent = new char[scriptSize + 1];
        rewind(scriptIn);
        fread(scriptContent, sizeof(char), scriptSize + 1, scriptIn);
        scriptContent[scriptSize] = '\0';
        fclose(scriptIn);
        auto rs = 0;
        if ((rs = _luaL_loadstring(L, (const char*)scriptContent)) == 0)
            rs = ori_lua_pcall(L, 0, LUA_MULTRET, 0);
        if (!Validate(L, rs))
            return;
        delete[] scriptContent;
    }

    DWORD GetPositionAddress() {
        return Lua_GetPositionAddress();
    }
}
