#pragma once
#include "framework.h"
#include <vector>

#include "../Common/MinHook.h"
#include "macro.h"

namespace core::directinputhook {
    DLLEXPORT bool PopulateMethodRVAs();
    std::vector<common::minhook::HookConfig> HookConfig();
}
