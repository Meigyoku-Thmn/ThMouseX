#pragma once
#include "framework.h"
#include <vector>

#include "../Common/MinHook.h"
#include "macro.h"

#include "DllLoad.h"

namespace dx8::hook {
    using CallbackType = void (*)(void);
    EXPORT_FUNC(void, RegisterPostRenderCallbacks, CallbackType callback);
    EXPORT_FUNC(bool, PopulateMethodRVAs);
    EXPORT_FUNC(std::vector<common::minhook::HookConfig>, HookConfig);
    EXPORT_FUNC(void, ClearMeasurementFlags);
}