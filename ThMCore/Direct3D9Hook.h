#pragma once
#include "framework.h"
#include <vector>

#include "../Common/MinHook.h"
#include "macro.h"

namespace core::directx9hook {
    using namespace std;

    extern bool measurementPrepared;
    extern bool cursorStatePrepared;

    using CallbackType = void (*)(void);
    DLLEXPORT void RegisterPostRenderCallbacks(CallbackType callback);
    DLLEXPORT bool PopulateMethodRVAs();
    vector<common::minhook::HookConfig> HookConfig();
}
