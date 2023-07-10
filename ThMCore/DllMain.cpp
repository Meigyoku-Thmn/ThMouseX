#include "framework.h"
#include <shlwapi.h>
#include <clocale>

#include "../Common/macro.h"
#include "../Common/MinHook.h"
#include "../Common/CallbackStore.h"
#include "../Common/Variables.h"
#include "../Common/LuaJIT.h"
#include "KeyboardStateHook.h"
#include "MessageQueueHook.h"
#include "DirectInputHook.h"
#include "Direct3D8Hook.h"
#include "Direct3D9Hook.h"
#include "Direct3D11Hook.h"

namespace minhook = common::minhook;
namespace callbackstore = common::callbackstore;
namespace luajit = common::luajit;
namespace messagequeuehook = core::messagequeuehook;
namespace directx8 = core::directx8hook;
namespace directx9 = core::directx9hook;
namespace directx11 = core::directx11hook;
namespace directinput = core::directinputhook;
namespace keyboardstate = core::keyboardstatehook;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            g_coreModule = hModule;
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            if (g_hookApplied)
                callbackstore::TriggerUninitializeCallbacks(lpReserved != 0);
            break;
    }
    return TRUE;
}

