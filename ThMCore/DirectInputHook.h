#pragma once
#include "framework.h"
#include <vector>

#include "../Common/MinHook.h"
#include "macro.h"

namespace core::directinputhook {
    using namespace std;

    DLLEXPORT bool PopulateMethodRVAs();
    vector<common::minhook::HookConfig> HookConfig();
}
