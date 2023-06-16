#pragma once
#include "framework.h"
#include <vector>

#include "../Common/MinHook.h"
#include "macro.h"

#include "DllLoad.h"

namespace dx8::hook {
    using CallbackType = void (*)(void);
    RUNTIME_EXPORT_FUNC(void, RegisterPostRenderCallbacks, CallbackType callback);
    RUNTIME_EXPORT_FUNC(bool, PopulateMethodRVAs);
    RUNTIME_EXPORT_FUNC(std::vector<common::minhook::HookConfig>, HookConfig);
    RUNTIME_EXPORT_FUNC(void, ClearMeasurementFlags);
}