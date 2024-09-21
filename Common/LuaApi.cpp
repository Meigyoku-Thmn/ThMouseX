#include "LuaApi.h"
#include "Helper.h"
#include "Helper.Memory.h"
#include "Helper.Encoding.h"
#include "Log.h"
#include "Variables.h"
#include "CallbackStore.h"
#include "MinHook.h"
#include <MinHook.h>
#include "../ThMouseX/resource.h"

#include <Windows.h>
#include <string>
#include <format>
#include <span>

namespace note = common::log;
namespace helper = common::helper;
namespace memory = common::helper::memory;
namespace encoding = common::helper::encoding;
namespace callbackstore = common::callbackstore;
namespace minhook = common::minhook;

using namespace std;

MH_STATUS Lua_CreateHookApi(LPCSTR pszModule, LPCSTR pszProcName, LPVOID pDetour, LPVOID* ppOriginal) {
    return MH_CreateHookApi(encoding::ConvertToUtf16(pszModule).c_str(), pszProcName, pDetour, ppOriginal);
}

DWORD Lua_ReadUInt32(DWORD address) {
    return *PDWORD(address);
}

DWORD Lua_ResolveAddress(DWORD* offsets, size_t length) {
    return memory::ResolveAddress(span{ offsets, length });
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

PointDataType Lua_GetDataType() {
    return g_gameConfig.PosDataType;
}

void Lua_RegisterUninitializeCallback(callbackstore::UninitializeCallbackType callback) {
    callbackstore::RegisterUninitializeCallback(callback, true);
}

string LuaJitPrepScript;
ON_INIT{
    auto dllModule = HINST_THISCOMPONENT;
    auto scriptRes = FindResourceW(dllModule, MAKEINTRESOURCEW(LUAJIT_PREP_SCRIPT), L"LUASCRIPT");
    if (scriptRes == nil)
        return;
    auto scriptSize = SizeofResource(dllModule, scriptRes);
    auto scriptHandle = LoadResource(dllModule, scriptRes);
    if (scriptHandle == nil)
        return;
    LuaJitPrepScript = string((const char*)LockResource(scriptHandle), scriptSize);
};

namespace common::luaapi {
    static void Uninitialize(bool isProcessTerminating) {
        Lua_SetPositionAddress(NULL);
    }

    void Initialize() {
        callbackstore::RegisterUninitializeCallback(Uninitialize);
    }

    string MakePreparationScript() {
        auto thisDllPath = encoding::ConvertToUtf8(wstring(g_currentModuleDirPath) + L"\\" + L_(APP_NAME));
        helper::Replace(thisDllPath, "\\", R"(\\)");
        auto moduleHandle = uintptr_t(g_coreModule);
        return vformat(LuaJitPrepScript, make_format_args(thisDllPath, moduleHandle));
    }
}
