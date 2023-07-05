#pragma once
#include <vector>
#include "macro.h"
#include "framework.h"

#include "DllLoad.h"

namespace common::minhook {
    struct HookConfig {
        LPVOID  pTarget;
        LPVOID  pDetour;
        LPVOID* ppOriginal;
    };

    struct HookApiConfig {
        LPCWSTR moduleName;
        LPCSTR  procName;
        LPVOID  pDetour;
        LPVOID* ppOriginal;
    };

    using CallbackType = void (*)(bool isProcessTerminating);
    EXPORT_FUNC(void, MinHook_, RegisterUninitializeCallback, CallbackType callback);
    EXPORT_FUNC(bool, MinHook_, Initialize);
    EXPORT_FUNC(bool, MinHook_, CreateHook, const std::vector<HookConfig>& hookConfigs);
    EXPORT_FUNC(bool, MinHook_, CreateApiHook, const std::vector<HookApiConfig>& hookConfigs);
    EXPORT_FUNC(bool, MinHook_, EnableHooks, const std::vector<HookApiConfig>& hookConfigs);
    EXPORT_FUNC(bool, MinHook_, DisableHooks, const std::vector<HookApiConfig>& hookConfigs);
    EXPORT_FUNC(bool, MinHook_, EnableAll);
    EXPORT_FUNC(void, MinHook_, Uninitialize, bool isProcessTerminating);
}
