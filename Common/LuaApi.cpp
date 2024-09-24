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
#include <span>

namespace note = common::log;
namespace helper = common::helper;
namespace memory = common::helper::memory;
namespace encoding = common::helper::encoding;
namespace callbackstore = common::callbackstore;
namespace minhook = common::minhook;

using namespace std;

static DWORD positionAddress;
namespace common::luaapi {
    string LuaJitPrepScript;

    static void Uninitialize(bool isProcessTerminating) {
        SetPositionAddress(NULL);
    }

    void Initialize() {
        callbackstore::RegisterUninitializeCallback(Uninitialize);
        auto dllModule = HINST_THISCOMPONENT;
        auto scriptRes = FindResourceW(dllModule, MAKEINTRESOURCEW(LUAJIT_PREP_SCRIPT_NAME), L_(LUAJIT_PREP_SCRIPT_TYPE));
        if (scriptRes == nil)
            return;
        auto scriptSize = SizeofResource(dllModule, scriptRes);
        auto scriptHandle = LoadResource(dllModule, scriptRes);
        if (scriptHandle == nil)
            return;
        LuaJitPrepScript = string((const char*)LockResource(scriptHandle), scriptSize);
    }

    DWORD GetPositionAddress() {
        return positionAddress;
    }

    void SetPositionAddress(DWORD address) {
        positionAddress = address;
    }

    DWORD ReadUInt32(DWORD address) {
        return *PDWORD(address);
    }

    DWORD ResolveAddress(DWORD* offsets, size_t length) {
        return memory::ResolveAddress(span{ offsets, length });
    }

    void OpenConsole() {
        note::OpenConsole();
    }

    PointDataType GetDataType() {
        return g_gameConfig.PosDataType;
    }

    void RegisterUninitializeCallback(UninitializeCallbackType callback) {
        callbackstore::RegisterUninitializeCallback(callback, true);
    }

    MH_STATUS CreateHookApi(LPCSTR pszModule, LPCSTR pszProcName, LPVOID pDetour, LPVOID* ppOriginal) {
        return MH_CreateHookApi(encoding::ConvertToUtf16(pszModule).c_str(), pszProcName, pDetour, ppOriginal);
    }
}
