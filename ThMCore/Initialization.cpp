#include "Initialization.h"

#include "framework.h"
#include <shlwapi.h>
#include <clocale>
#include <vector>

#include "../Common/macro.h"
#include "../Common/MinHook.h"
#include "../Common/CallbackStore.h"
#include "../Common/Variables.h"
#include "../Common/LuaJIT.h"
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
namespace luajit = common::luajit;
namespace neolua = common::neolua;
namespace messagequeue = core::messagequeue;
namespace sendkey = core::sendkey;
namespace directx8 = core::directx8;
namespace directx9 = core::directx9;
namespace directx11 = core::directx11;
namespace directinput = core::directinput;
namespace keyboardstate = core::keyboardstate;

namespace core {
    HMODULE WINAPI _LoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
    decltype(&_LoadLibraryExW) OriLoadLibraryExW;

    void Initialize() {
        setlocale(LC_ALL, ".UTF8");
        setlocale(LC_NUMERIC, "C");

        g_targetModule = GetModuleHandleW(NULL);

        GetModuleFileNameW(g_coreModule, g_currentModuleDirPath, ARRAYSIZE(g_currentModuleDirPath));
        PathRemoveFileSpecW(g_currentModuleDirPath);

        WCHAR currentProcessName[MAX_PATH];
        GetModuleFileNameW(g_targetModule, currentProcessName, ARRAYSIZE(currentProcessName));
        PathStripPathW(currentProcessName);
        PathRemoveExtensionW(currentProcessName);

        if (_wcsicmp(currentProcessName, L"ThMouseX") == 0)
            return;

        auto& gameConfigs = gs_gameConfigArray;
        for (int i = 0; i < gameConfigs.Length; i++) {
            if (_wcsicmp(currentProcessName, gameConfigs.Configs[i].ProcessName) == 0) {
                g_currentConfig = gameConfigs.Configs[i];

                minhook::Initialize();
                luajit::Initialize();
                messagequeue::Initialize();
                neolua::Initialize();
                sendkey::Initialize();

                directx8::Initialize();
                directx9::Initialize();
                directx11::Initialize();

                directinput::Initialize();
                keyboardstate::Initialize();
                messagequeue::Initialize();

                std::vector<minhook::HookApiConfig> hookConfigs = {
                    { L"KERNELBASE.dll", "LoadLibraryExW", &_LoadLibraryExW, (PVOID*)&OriLoadLibraryExW },
                };
                minhook::CreateApiHook(hookConfigs);

                minhook::EnableAll();
                g_hookApplied = true;

                break;
            }
        }
    }

    HMODULE WINAPI _LoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
        auto rs = OriLoadLibraryExW(lpLibFileName, hFile, dwFlags);
        if (rs != NULL) {
            auto fileName = PathFindFileNameW(lpLibFileName);
            if (_wcsicmp(fileName, L"d3d8.dll") == 0)
                directx8::Initialize();
            else if (_wcsicmp(fileName, L"d3d9.dll") == 0)
                directx9::Initialize();
            else if (_wcsicmp(fileName, L"d3d11.dll") == 0)
                directx11::Initialize();
            else if (_wcsicmp(fileName, L"DInput8.dll") == 0)
                directinput::Initialize();
        }
        return rs;
    }

    void Uninitialize(bool isProcessTerminating) {
        if (g_hookApplied)
            callbackstore::TriggerUninitializeCallbacks(isProcessTerminating);
    }
}