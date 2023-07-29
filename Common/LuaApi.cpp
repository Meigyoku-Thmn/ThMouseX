#include "LuaApi.h"
#include "Helper.h"
#include "Helper.Memory.h"
#include "Helper.Encoding.h"
#include "Log.h"
#include "Variables.h"
#include "CallbackStore.h"
#include "resource.h"

#include <Windows.h>
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

string LuaJitPrepScript;
ON_INIT{
    auto dllModule = GetModuleHandleW(L_(APP_NAME".dll"));
    auto scriptRes = FindResourceW(dllModule, MAKEINTRESOURCEW(LUAJIT_PREP_SCRIPT), L"LUASCRIPT");
    if (scriptRes == NULL)
        return;
    auto scriptSize = SizeofResource(dllModule, scriptRes);
    auto scriptHandle = LoadResource(dllModule, scriptRes);
    if (scriptHandle == NULL)
        return;
    LuaJitPrepScript = string((const char*)LockResource(scriptHandle), scriptSize);
};

string MakePreparationScriptForLuaJIT() {
    auto thisDllPath = encoding::ConvertToUtf8((wstring(g_currentModuleDirPath) + L"\\" + L_(APP_NAME)).c_str());
    helper::Replace(thisDllPath, "\\", "\\\\");
    return vformat(LuaJitPrepScript, make_format_args(thisDllPath));
}