#pragma once
#include "framework.h"
#include <vector>

#include "../Common/MinHook.h"
#include "macro.h"

namespace core::messagequeuehook {
    using namespace std;

    DLLEXPORT_C bool TestRegisteredWindowMessages();
    extern vector<common::minhook::HookApiConfig> HookConfig;
    DLLEXPORT_C bool InstallHooks();
    DLLEXPORT_C void RemoveHooks();
}