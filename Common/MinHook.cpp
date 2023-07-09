#include <MinHook.h>
#include <vector>
#include "macro.h"

#include "MinHook.h"

using namespace std;

namespace common::minhook {
    vector<CallbackType>& uninitializeCallbacks() {
        static vector<CallbackType> backing;
        return backing;
    }

    void RegisterUninitializeCallback(CallbackType callback) {
        uninitializeCallbacks().push_back(callback);
    }

    bool Initialize() {
        return MH_Initialize() == MH_OK;
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

    void Uninitialize(bool isProcessTerminating) {
        for (auto& callback : uninitializeCallbacks())
            callback(isProcessTerminating);
        MH_Uninitialize();
    }
}
