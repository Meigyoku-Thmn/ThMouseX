#pragma once
#include "framework.h"
#include <vector>

#include "../Common/MinHook.h"

namespace core::directx11hook {
    using CallbackType = void (*)(void);
    bool PopulateMethodRVAs();
    std::vector<common::minhook::HookConfig> HookConfig();
    void Initialize();
}
