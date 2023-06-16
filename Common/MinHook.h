#pragma once
#include <vector>
#include "macro.h"
#include "framework.h"

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
    DLLEXPORT void RegisterUninitializeCallback(CallbackType callback);
    DLLEXPORT bool Initialize();
    DLLEXPORT bool CreateHook(const std::vector<HookConfig>& hookConfigs);
    DLLEXPORT bool CreateHook(const std::vector<HookApiConfig>& hookConfigs);
    DLLEXPORT bool EnableAll();
    DLLEXPORT void Uninitialize(bool isProcessTerminating);
}
