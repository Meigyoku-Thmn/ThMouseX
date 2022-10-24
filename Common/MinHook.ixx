module;

#include "MinHook.h"
#include <vector>
#include "macro.h"

export module common.minhook;

using namespace std;

namespace common::minhook {
    export struct HookConfig {
        LPVOID  pTarget;
        LPVOID  pDetour;
        LPVOID* ppOriginal;
    };

    export struct HookApiConfig {
        LPCWSTR moduleName;
        LPCSTR  procName;
        LPVOID  pDetour;
        LPVOID* ppOriginal;
    };

    using CallbackType = void (*)(bool isProcessTerminating);
    vector<CallbackType>& uninitializeCallbacks() {
        static vector<CallbackType> backing;
        return backing;
    }
    export DLLEXPORT void RegisterUninitializeCallback(CallbackType callback) {
        uninitializeCallbacks().push_back(callback);
    }

    export DLLEXPORT bool Initialize() {
        return MH_Initialize() == MH_OK;
    }

    export DLLEXPORT bool CreateHook(const vector<HookConfig>& hookConfigs) {
        for (auto& config : hookConfigs) {
            auto rs = MH_CreateHook(config.pTarget, config.pDetour, config.ppOriginal);
            if (rs != MH_OK)
                return false;
        }
        return true;
    }

    export DLLEXPORT bool CreateHook(const vector<HookApiConfig>& hookConfigs) {
        for (auto& config : hookConfigs) {
            auto rs = MH_CreateHookApi(config.moduleName, config.procName, config.pDetour, config.ppOriginal);
            if (rs != MH_OK)
                return false;
        }
        return true;
    }

    export DLLEXPORT bool EnableAll() {
        return MH_EnableHook(MH_ALL_HOOKS) == MH_OK;
    }

    export DLLEXPORT void Uninitialize(bool isProcessTerminating) {
        for (auto& callback : uninitializeCallbacks())
            callback(isProcessTerminating);
        MH_Uninitialize();
    }
}
