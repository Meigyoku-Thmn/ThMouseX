#pragma once
#include "framework.h"
#include <vector>

#include "../Common/MinHook.h"

namespace core::directx9hook {
    using CallbackType = void (*)(void);
    bool PopulateMethodRVAs();
    std::vector<common::minhook::HookConfig> HookConfig();
}
