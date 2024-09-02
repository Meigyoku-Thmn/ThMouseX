#include "Initialization.h"

#include <Windows.h>
#include <shlwapi.h>
#include <clocale>
#include <vector>

#include "../Common/macro.h"
#include "../Common/Log.h"
#include "../Common/Helper.h"
#include "../Common/Helper.Encoding.h"
#include "../Common/Helper.Memory.h"
#include "../Common/MinHook.h"
#include "../Common/CallbackStore.h"
#include "../Common/Variables.h"
#include "../Common/LuaApi.h"
#include "../Common/LuaJIT.h"
#include "../Common/Lua.h"
#include "../Common/NeoLua.h"
#include "KeyboardState.h"
#include "SendKey.h"
#include "MessageQueue.h"
#include "DirectInput.h"
#include "Direct3D8.h"
#include "Direct3D9.h"
#include "Direct3D11.h"
#include "ComClient.h"

namespace minhook = common::minhook;
namespace callbackstore = common::callbackstore;
namespace luaapi = common::luaapi;
namespace luajit = common::luajit;
namespace neolua = common::neolua;
namespace lua = common::lua;
namespace messagequeue = core::messagequeue;
namespace sendkey = core::sendkey;
namespace directx8 = core::directx8;
namespace directx9 = core::directx9;
namespace directx11 = core::directx11;
namespace directinput = core::directinput;
namespace keyboardstate = core::keyboardstate;
namespace note = common::log;
namespace helper = common::helper;
namespace encoding = common::helper::encoding;
namespace memory = common::helper::memory;
namespace comclient = core::comclient;

namespace core {
    HMODULE WINAPI _LoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
    decltype(&_LoadLibraryExW) OriLoadLibraryExW;

    void Initialize() {
        setlocale(LC_ALL, ".UTF8");
        setlocale(LC_NUMERIC, "C");

        g_targetModule = GetModuleHandleW(nil);

        GetSystemDirectoryW(g_systemDirPath, ARRAYSIZE(g_systemDirPath));

        GetModuleFileNameW(g_coreModule, g_currentModuleDirPath, ARRAYSIZE(g_currentModuleDirPath));
        PathRemoveFileSpecW(g_currentModuleDirPath);

        GetModuleFileNameW(g_targetModule, g_currentProcessName, ARRAYSIZE(g_currentProcessName));
        wcscpy(g_currentProcessDirPath, g_currentProcessName);
        PathRemoveFileSpecW(g_currentProcessDirPath);
        PathStripPathW(g_currentProcessName);
        PathRemoveExtensionW(g_currentProcessName);

        if (helper::IsCurrentProcessThMouseX())
            return;

        if (!comclient::Initialize())
            return;

        GameConfig gameConfig;
        if (!comclient::GetGameConfig(g_currentProcessName, gameConfig))
            return;

        g_currentConfig = gameConfig;

        minhook::Initialize();
        luaapi::Initialize();
        lua::Initialize();
        luajit::Initialize();
        neolua::Initialize();

        directx11::Initialize();
        directx9::Initialize();
        directx8::Initialize();

        directinput::Initialize();
        sendkey::Initialize();
        keyboardstate::Initialize();
        messagequeue::Initialize();

        minhook::CreateApiHook(std::vector<minhook::HookApiConfig> {
            { L"KERNELBASE.dll", "LoadLibraryExW", &_LoadLibraryExW, &OriLoadLibraryExW },
        });

        minhook::EnableAll();
        g_hookApplied = true;
    }

    thread_local UINT loadLibraryReentrantCount = 0;
    HMODULE WINAPI _LoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
        auto allowInitialization = loadLibraryReentrantCount == 0;
        loadLibraryReentrantCount++;
        auto rs = OriLoadLibraryExW(lpLibFileName, hFile, dwFlags);
        loadLibraryReentrantCount--;
        if (allowInitialization) {
            directx11::Initialize();
            directx9::Initialize();
            directx8::Initialize();
            directinput::Initialize();
            minhook::EnableAll();
        }
        return rs;
    }

    void Uninitialize(bool isProcessTerminating) {
        if (g_hookApplied) {
            callbackstore::TriggerUninitializeCallbacks(isProcessTerminating);
        }
    }
}