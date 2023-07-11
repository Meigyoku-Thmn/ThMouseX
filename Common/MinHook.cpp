#include <MinHook.h>
#include <vector>
#include "macro.h"
#include <nameof.hpp>

#include "MinHook.h"
#include "CallbackStore.h"
#include "Log.h"

namespace note = common::log;

using namespace std;

namespace common::minhook {
    void Uninitialize(bool isProcessTerminating) {
        auto rs = MH_Uninitialize();
        auto errName = string(NAMEOF_ENUM(rs));
        if (!isProcessTerminating && rs != MH_OK)
            note::ToFile("Failed to uninitialize MinHook: %s", errName.c_str());
    }

    bool Initialize() {
        auto rs = MH_Initialize();
        auto errName = string(NAMEOF_ENUM(rs));
        if (rs != MH_OK) {
            note::ToFile("Failed to initialize MinHook: %s", errName.c_str());
            return false;
        }
        callbackstore::RegisterUninitializeCallback(Uninitialize);
        return true;
    }

    bool CreateHook(const vector<HookConfig>& hookConfigs) {
        for (auto& config : hookConfigs) {
            auto rs = MH_CreateHook(config.pTarget, config.pDetour, config.ppOriginal);
            auto errName = string(NAMEOF_ENUM(rs));
            if (rs != MH_OK) {
                note::ToFile("Failed to create hook for target %p: %s", config.pTarget, errName.c_str());
                return false;
            }
        }
        return true;
    }

    bool CreateApiHook(const vector<HookApiConfig>& hookConfigs) {
        for (auto& config : hookConfigs) {
            auto rs = MH_CreateHookApi(config.moduleName, config.procName, config.pDetour, config.ppOriginal);
            auto errName = string(NAMEOF_ENUM(rs));
            if (rs != MH_OK) {
                note::ToFile("Failed to create hook for api %s|%s: %s", config.moduleName, config.procName, errName.c_str());
                return false;
            }
        }
        return true;
    }

    bool EnableHooks(const vector<HookApiConfig>& hookConfigs) {
        for (auto& config : hookConfigs) {
            auto hModule = GetModuleHandleW(config.moduleName);
            if (!hModule)
                return false;
            auto proc = GetProcAddress(hModule, config.procName);
            if (!proc)
                return false;
            auto rs = MH_EnableHook(proc);
            if (rs != MH_OK)
                return false;
        }
        return true;
    }

    bool DisableHooks(const vector<HookApiConfig>& hookConfigs) {
        for (auto& config : hookConfigs) {
            auto hModule = GetModuleHandleW(config.moduleName);
            if (!hModule)
                return false;
            auto proc = GetProcAddress(hModule, config.procName);
            if (!proc)
                return false;
            auto rs = MH_DisableHook(proc);
            if (rs != MH_OK)
                return false;
        }
        return true;
    }

    bool EnableAll() {
        auto rs = MH_EnableHook(MH_ALL_HOOKS);
        auto errName = string(NAMEOF_ENUM(rs));
        if (rs != MH_OK) {
            note::ToFile("Failed to enable all hooks: %s", errName.c_str());
            return false;
        }
    }
}
