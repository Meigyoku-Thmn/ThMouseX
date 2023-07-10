#include <MinHook.h>
#include <vector>
#include "macro.h"

#include "MinHook.h"
#include "CallbackStore.h"

using namespace std;

namespace common::minhook {
    void Uninitialize(bool isProcessTerminating) {
        MH_Uninitialize();
    }

    bool Initialize() {
        if (MH_Initialize() != MH_OK)
            return false;
        callbackstore::RegisterUninitializeCallback(Uninitialize);
        return true;
    }

    bool CreateHook(const vector<HookConfig>& hookConfigs) {
        for (auto& config : hookConfigs) {
            auto rs = MH_CreateHook(config.pTarget, config.pDetour, config.ppOriginal);
            if (rs != MH_OK)
                return false;
        }
        return true;
    }

    bool CreateApiHook(const vector<HookApiConfig>& hookConfigs) {
        for (auto& config : hookConfigs) {
            auto rs = MH_CreateHookApi(config.moduleName, config.procName, config.pDetour, config.ppOriginal);
            if (rs != MH_OK)
                return false;
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
        return MH_EnableHook(MH_ALL_HOOKS) == MH_OK;
    }
}
