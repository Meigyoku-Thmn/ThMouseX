#pragma once
#include "framework.h"
#include <vector>

#include "../Common/MinHook.h"
#include "../Common/macro.h"

namespace core::messagequeuehook {
    DLLEXPORT_C bool TestRegisteredWindowMessages();
    extern std::vector<common::minhook::HookApiConfig> HookConfig;
    DLLEXPORT_C bool InstallHooks();
    DLLEXPORT_C void RemoveHooks();
}