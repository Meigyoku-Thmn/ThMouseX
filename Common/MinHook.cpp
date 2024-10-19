#include <MinHook.h>
#include <vector>
#include "macro.h"

#include "MinHook.h"
#include "CallbackStore.h"
#include "Log.h"
#include "Helper.Encoding.h"

#define ALL_HOOKS nullptr

namespace note = common::log;
namespace encoding = common::helper::encoding;

using namespace std;

namespace common::minhook {
    static void Uninitialize(bool isProcessTerminating) {
        auto rs = MH_Uninitialize(HOOK_ENGINE_STATE_NAME);
        if (!isProcessTerminating && rs != MH_OK)
            note::ToFile("[MinHook] Failed to uninitialize MinHook: %s", MH_StatusToString(rs));
    }

    bool Initialize() {
        if (auto rs = MH_Initialize(HOOK_ENGINE_STATE_NAME); rs != MH_OK) {
            note::ToFile("[MinHook] Failed to initialize MinHook: %s", MH_StatusToString(rs));
            return false;
        }
        callbackstore::RegisterUninitializeCallback(Uninitialize);
        return true;
    }

    bool CreateHook(const vector<HookConfig>& hookConfigs) {
        for (auto& config : hookConfigs) {
            auto rs = MH_CreateHook(config.pTarget, config.pDetour, (LPVOID*)config.ppOriginal, config.discriminator);
            if (rs != MH_OK && rs != MH_ERROR_ALREADY_CREATED) {
                note::ToFile("[MinHook] Failed to create hook for target %p: %s", config.pTarget, MH_StatusToString(rs));
                return false;
            }
        }
        return true;
    }

    bool CreateApiHook(const vector<HookApiConfig>& hookConfigs) {
        for (auto& config : hookConfigs) {
            auto rs = MH_CreateHookApi(config.moduleName, config.procName, config.pDetour, (LPVOID*)config.ppOriginal, config.discriminator);
            if (rs != MH_OK && rs != MH_ERROR_ALREADY_CREATED) {
                auto moduleName = encoding::ConvertToUtf8(config.moduleName);
                note::ToFile("[MinHook] Failed to create hook for api %s|%s: %s", moduleName.c_str(), config.procName, MH_StatusToString(rs));
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
            auto rs = MH_EnableHook(proc, config.discriminator);
            if (rs != MH_OK)
                return false;
        }
        return true;
    }

    bool EnableHooks(const vector<HookConfig>& hookConfigs) {
        for (auto& config : hookConfigs) {
            auto rs = MH_EnableHook(config.pTarget, config.discriminator);
            if (rs != MH_OK)
                return false;
        }
        return true;
    }

    bool EnableAll() {
        auto rs = MH_EnableHook(ALL_HOOKS);
        if (rs != MH_OK) {
            note::ToFile("[MinHook] Failed to enable all hooks: %s", MH_StatusToString(rs));
            return false;
        }
        return true;
    }

    bool DisableHooks(const std::vector<HookApiConfig>& hookConfigs) {
        for (auto& config : hookConfigs) {
            auto hModule = GetModuleHandleW(config.moduleName);
            if (!hModule)
                return false;
            auto proc = GetProcAddress(hModule, config.procName);
            if (!proc)
                return false;
            auto rs = MH_DisableHook(proc, config.discriminator);
            if (rs != MH_OK)
                return false;
        }
        return true;
    }

    bool DisableHooks(const std::vector<HookConfig>& hookConfigs) {
        for (auto& config : hookConfigs) {
            auto rs = MH_DisableHook(config.pTarget, config.discriminator);
            if (rs != MH_OK)
                return false;
        }
        return true;
    }

    bool DisableAll() {
        auto rs = MH_DisableHook(ALL_HOOKS);
        if (rs != MH_OK) {
            note::ToFile("[MinHook] Failed to enable all hooks: %s", MH_StatusToString(rs));
            return false;
        }
        return true;
    }
}
