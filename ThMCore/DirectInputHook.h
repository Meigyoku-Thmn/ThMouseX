#pragma once
#include "framework.h"
#include <vector>

#include "../Common/MinHook.h"
#include "macro.h"

namespace core::directinputhook {
    bool PopulateMethodRVAs();
    std::vector<common::minhook::HookConfig> HookConfig();
}
