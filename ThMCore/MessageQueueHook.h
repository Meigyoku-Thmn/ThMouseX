#pragma once
#include "framework.h"
#include <vector>

#include "../Common/MinHook.h"
#include "macro.h"

namespace core::messagequeuehook {
    using namespace std;

    DLLEXPORT bool TestRegisteredWindowMessages();
    extern vector<common::minhook::HookApiConfig> HookConfig;
    DLLEXPORT bool InstallHooks();
    DLLEXPORT void RemoveHooks();
}