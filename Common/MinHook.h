#pragma once
#include <vector>
#include "macro.h"
#include <Windows.h>

namespace common::minhook {
    struct HookConfig {
        LPVOID  pTarget;
        LPVOID  pDetour;
        LPVOID  ppOriginal;
        PCCH    discriminator;
    };

    struct HookApiConfig {
        LPCWSTR moduleName;
        LPCSTR  procName;
        LPVOID  pDetour;
        LPVOID  ppOriginal;
        PCCH    discriminator;
    };

    bool Initialize();
    bool CreateHook(const std::vector<HookConfig>& hookConfigs);
    bool CreateApiHook(const std::vector<HookApiConfig>& hookConfigs);
    bool EnableHooks(const std::vector<HookApiConfig>& hookConfigs);
    bool EnableHooks(const std::vector<HookConfig>& hookConfigs);
    bool EnableAll();
    bool DisableHooks(const std::vector<HookApiConfig>& hookConfigs);
    bool DisableHooks(const std::vector<HookConfig>& hookConfigs);
    bool DisableAll();
}
