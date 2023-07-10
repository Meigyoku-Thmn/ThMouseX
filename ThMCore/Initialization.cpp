#include "Initialization.h"

#include "framework.h"
#include <shlwapi.h>
#include <clocale>

#include "../Common/macro.h"
#include "../Common/MinHook.h"
#include "../Common/CallbackStore.h"
#include "../Common/Variables.h"
#include "../Common/LuaJIT.h"
#include "../Common/NeoLua.h"
#include "KeyboardStateHook.h"
#include "MessageQueueHook.h"
#include "DirectInputHook.h"
#include "Direct3D8Hook.h"
#include "Direct3D9Hook.h"
#include "Direct3D11Hook.h"

namespace minhook = common::minhook;
namespace callbackstore = common::callbackstore;
namespace luajit = common::luajit;
namespace neolua = common::neolua;
namespace messagequeuehook = core::messagequeuehook;
namespace directx8 = core::directx8hook;
namespace directx9 = core::directx9hook;
namespace directx11 = core::directx11hook;
namespace directinput = core::directinputhook;
namespace keyboardstate = core::keyboardstatehook;

namespace core {
    void Initialize() {
        setlocale(LC_ALL, ".UTF8");
        setlocale(LC_NUMERIC, "C");

        g_targetModule = GetModuleHandleW(NULL);

        GetModuleFileNameW(g_coreModule, g_currentModuleDirPath, ARRAYSIZE(g_currentModuleDirPath));
        g_currentModuleDirPath[ARRAYSIZE(g_currentModuleDirPath) - 1] = '\0';
        PathRemoveFileSpecW(g_currentModuleDirPath);

        WCHAR currentProcessName[MAX_PATH + 1];

        GetModuleFileNameW(g_targetModule, currentProcessName, ARRAYSIZE(currentProcessName));
        currentProcessName[ARRAYSIZE(currentProcessName) - 1] = '\0';
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
                neolua::Initialize();

                minhook::CreateHook(directx8::HookConfig());
                minhook::CreateHook(directx9::HookConfig());
                minhook::CreateHook(directx11::HookConfig());

                minhook::CreateHook(directinput::HookConfig());
                minhook::CreateApiHook(keyboardstate::HookConfig());

                minhook::CreateApiHook(messagequeuehook::HookConfig);

                minhook::EnableAll();
                g_hookApplied = true;

                break;
            }
        }
    }
}