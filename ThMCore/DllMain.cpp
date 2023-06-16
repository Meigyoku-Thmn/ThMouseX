#include "framework.h"
#include <shlwapi.h>
#include <clocale>

#include "../Common/MinHook.h"
#include "../Common/Variables.h"
#include "../Common/LuaJIT.h"
#include "../DX8Hook/Direct3D8Hook.h"
#include "KeyboardStateHook.h"
#include "MessageQueueHook.h"
#include "DirectInputHook.h"
#include "Direct3D9Hook.h"
#include "macro.h"

namespace minhook = common::minhook;
namespace luajit = common::luajit;
namespace messagequeuehook = core::messagequeuehook;
namespace directx8 = dx8::hook;
namespace directx9 = core::directx9hook;
namespace directinput = core::directinputhook;
namespace keyboardstate = core::keyboardstatehook;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH: {
            setlocale(LC_ALL, ".UTF8");
            setlocale(LC_NUMERIC, "C");

            g_coreModule = hModule;
            g_targetModule = GetModuleHandleW(NULL);
            // We don't need thread notifications for what we're doing.
            // Thus, get rid of them, thereby eliminating some of the overhead of this DLL.
            DisableThreadLibraryCalls(hModule);

            WCHAR currentProcessName[MAX_PATH + 1];
            // Only hook the APIs if we have a configuation of the process.
            // If the process is not what we have anything to do with, just return TRUE, no need to eagerly unload.
            // The DLL will be forcefully unloaded from all processes when ThMouseX closes.
            GetModuleFileNameW(g_targetModule, currentProcessName, ARRAYSIZE(currentProcessName));
            currentProcessName[ARRAYSIZE(currentProcessName) - 1] = '\0';
            PathStripPathW(currentProcessName);
            PathRemoveExtensionW(currentProcessName);

            // Of course we ignore the mother.
            if (_wcsicmp(currentProcessName, L"THMouseX") == 0)
                return TRUE;

            // We use SetWindowsHookEx, so DllMain is always called from a message loop in the target process.
            // Therefore, this is pretty thread-safe unless you hook GetMessage/PeekMessage.
            // The same thing happens for UnhookWindowsHookEx.
            auto& gameConfigs = gs_gameConfigArray;
            for (int i = 0; i < gameConfigs.Length; i++) {
                if (_wcsicmp(currentProcessName, gameConfigs.Configs[i].ProcessName) == 0) {
                    g_currentConfig = gameConfigs.Configs[i];

                    luajit::Initialize();
                    minhook::Initialize();

                    // hook DirectX 8 for crosshair cursor and collect window measurement
                    minhook::CreateHook(directx8::HookConfig());

                    // hook DirectX 9 for crosshair cursor and collect window measurement
                    minhook::CreateHook(directx9::HookConfig());

                    // hook DirectInput8 for input manipulation
                    minhook::CreateHook(directinput::HookConfig());

                    // hook Joypad and GetKeyboardState for input manipulation
                    minhook::CreateApiHook(keyboardstate::HookConfig());

                    // hook Message Queue for additional runtime configuration
                    minhook::CreateApiHook(messagequeuehook::HookConfig);


                    minhook::EnableAll();
                    g_hookApplied = true;

                    break;
                }
            }
            break;
        }
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            if (g_hookApplied) {
                minhook::Uninitialize(lpReserved != 0);
                luajit::Uninitialize();
            }
            break;
    }
    return TRUE;
}

