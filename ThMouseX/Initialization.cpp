#include "Initialization.h"

#include "framework.h"
#include <shlwapi.h>
#include <clocale>
#include <vector>

#include "../Common/macro.h"
#include "../Common/Log.h"
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
namespace encoding = common::helper::encoding;
namespace memory = common::helper::memory;

#define LOAD_LIBRARY_AS_RES (LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE | LOAD_LIBRARY_AS_IMAGE_RESOURCE)

#define InitializeAndEnableHook(initializeBlock) initializeBlock; minhook::EnableAll(); 0
#define TryMatchAndInitializeModuleA(moduleName, ...) TryMatchAndInitializeModule(moduleName, __VA_ARGS__, _str,  )
#define TryMatchAndInitializeModuleW(moduleName, ...) TryMatchAndInitializeModule(moduleName, __VA_ARGS__, _wcs, L)
#define TryMatchAndInitializeModule(moduleName, elseBlock, cmpPrefix, strPrefix)    \
    if (cmpPrefix##icmp(moduleName, strPrefix"d3d8.dll") == 0) {                    \
        InitializeAndEnableHook(directx8::Initialize());                            \
    }                                                                               \
    else if (cmpPrefix##icmp(moduleName, strPrefix"d3d9.dll") == 0) {               \
        InitializeAndEnableHook(directx9::Initialize());                            \
    }                                                                               \
    else if (cmpPrefix##icmp(moduleName, strPrefix"d3d11.dll") == 0) {              \
        InitializeAndEnableHook(directx11::Initialize());                           \
    }                                                                               \
    else if (cmpPrefix##icmp(moduleName, strPrefix"DInput8.dll") == 0) {            \
        InitializeAndEnableHook(directinput::Initialize());                         \
    }                                                                               \
    else {                                                                          \
        elseBlock;                                                                  \
    }                                                                               

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

        WCHAR currentProcessName[MAX_PATH];
        GetModuleFileNameW(g_targetModule, currentProcessName, ARRAYSIZE(currentProcessName));
        wcscpy(g_currentProcessDirPath, currentProcessName);
        PathRemoveFileSpecW(g_currentProcessDirPath);
        PathStripPathW(currentProcessName);
        PathRemoveExtensionW(currentProcessName);

        if (_wcsicmp(currentProcessName, L_(APP_NAME)) == 0)
            return;
        if (_wcsicmp(currentProcessName, L_(APP_NAME "GUI")) == 0)
            return;

        for (auto ord = gs_gameConfigs.length(); ord > 0; ord--) {
            auto i = ord - 1;
            if (_wcsicmp(currentProcessName, gs_gameConfigs[i].ProcessName) == 0) {
                g_currentConfig = gs_gameConfigs[i];

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

                break;
            }
        }
    }

    HMODULE WINAPI _LoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
        auto rs = OriLoadLibraryExW(lpLibFileName, hFile, dwFlags);
        if (rs != nil && !(dwFlags & LOAD_LIBRARY_AS_RES)) {
            auto fileName = PathFindFileNameW(lpLibFileName);
            TryMatchAndInitializeModuleW(fileName, memory::ScanImportTable(rs, [](auto dllName) {
                TryMatchAndInitializeModuleA(dllName, 0);
            }));
        }
        return rs;
    }

    void Uninitialize(bool isProcessTerminating) {
        if (g_hookApplied) {
            callbackstore::TriggerUninitializeCallbacks(isProcessTerminating);
        }
    }
}