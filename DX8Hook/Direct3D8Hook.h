#pragma once
#include "framework.h"
#include <vector>

#include "../Common/MinHook.h"
#include "macro.h"

namespace dx8::hook {
    using namespace std;

    using CallbackType = void (*)(void);
    DLLEXPORT void RegisterPostRenderCallbacks(CallbackType callback);
    DLLEXPORT bool PopulateMethodRVAs();
    DLLEXPORT vector<common::minhook::HookConfig> HookConfig();
    DLLEXPORT void ClearMeasurementFlags();
}