#pragma once
#include <vector>
#include "macro.h"
#include "framework.h"

namespace common::minhook {
    struct HookConfig {
        LPVOID  pTarget;
        LPVOID  pDetour;
        LPVOID  ppOriginal;
    };

    struct HookApiConfig {
        LPCWSTR moduleName;
        LPCSTR  procName;
        LPVOID  pDetour;
        LPVOID  ppOriginal;
    };

    bool Initialize();
    bool CreateHook(const std::vector<HookConfig>& hookConfigs);
    bool CreateApiHook(const std::vector<HookApiConfig>& hookConfigs);
    bool EnableHooks(const std::vector<HookApiConfig>& hookConfigs);
    bool EnableHooks(const std::vector<HookConfig>& hookConfigs);
    bool RemoveHooks(const std::vector<HookApiConfig>& hookConfigs);
    bool RemoveHooks(const std::vector<HookConfig>& hookConfigs);
    bool EnableAll();
}
