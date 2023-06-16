#pragma once
#include "framework.h"
#include <vector>

#include "../Common/MinHook.h"
#include "macro.h"

namespace core::directx9hook {
    using CallbackType = void (*)(void);
    DLLEXPORT void RegisterPostRenderCallbacks(CallbackType callback);
    DLLEXPORT bool PopulateMethodRVAs();
    std::vector<common::minhook::HookConfig> HookConfig();
    DLLEXPORT void ClearMeasurementFlags();
}
