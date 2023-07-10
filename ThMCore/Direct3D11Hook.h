#pragma once
#include "framework.h"
#include <vector>

#include "../Common/MinHook.h"

namespace core::directx11hook {
    using CallbackType = void (*)(void);
    void RegisterPostRenderCallbacks(CallbackType callback);
    bool PopulateMethodRVAs();
    std::vector<common::minhook::HookConfig> HookConfig();
    void ClearMeasurementFlags();
}
