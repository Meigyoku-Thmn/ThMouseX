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
                messagequeue::Initialize();
                neolua::Initialize();
                sendkey::Initialize();

                directx8::Initialize();
                directx9::Initialize();
                directx11::Initialize();

                directinput::Initialize();
                keyboardstate::Initialize();
                messagequeue::Initialize();

                minhook::EnableAll();
                g_hookApplied = true;

                break;
            }
        }
    }

    void Uninitialize(bool isProcessTerminating) {
        if (g_hookApplied)
            callbackstore::TriggerUninitializeCallbacks(isProcessTerminating);
    }
}