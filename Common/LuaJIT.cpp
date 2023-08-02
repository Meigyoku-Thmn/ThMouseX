#include "framework.h"
#include <string>
#include <codecvt>
#include <luajit/lua.hpp>
#include <format>

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

#define GET_POSITION_ADDRESS "getPositionAddress"

static lua_State* L;

bool CheckAndDisableIfError(lua_State* L, int r) {
    if (r != 0) {
        note::ToFile("[LuaJIT] %s", lua_tostring(L, -1));
        lua_pop(L, 1);
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
            return g_coreModule;
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

        if (!CheckAndDisableIfError(L, luaL_dostring(L, luaapi::MakePreparationScript().c_str()))) {
            note::ToFile("[LuaJIT] The above error occurred in Preparation Script.");
            return;
        }

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
            return Lua_GetPositionAddress();
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
