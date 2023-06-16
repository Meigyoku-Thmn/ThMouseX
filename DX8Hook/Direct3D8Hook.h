#pragma once
#include "framework.h"
#include <vector>

#include "../Common/MinHook.h"
#include "macro.h"

namespace dx8::hook {
    using namespace std;

    DLLEXPORT extern bool measurementPrepared;
    DLLEXPORT extern bool cursorStatePrepared;

    using CallbackType = void (*)(void);
    DLLEXPORT void RegisterPostRenderCallbacks(CallbackType callback);
    DLLEXPORT bool PopulateMethodRVAs();
    DLLEXPORT vector<common::minhook::HookConfig> HookConfig();
}