module;

#include "MinHook.h"
#include <vector>
#include "macro.h"

export module common.minhook;

using namespace std;

export struct MHookConfig {
    LPVOID  pTarget;
    LPVOID  pDetour;
    LPVOID* ppOriginal;
};

export struct MHookApiConfig {
    LPCWSTR moduleName;
    LPCSTR  procName;
    LPVOID  pDetour;
    LPVOID* ppOriginal;
};

using CallbackType = void (*)(void);
vector<CallbackType>& uninitializeCallbacks() {
    static vector<CallbackType> backing;
    return backing;
}
export DLLEXPORT void RegisterMHookUninitializeCallback(CallbackType callback) {
    uninitializeCallbacks().push_back(callback);
}

export DLLEXPORT bool MHook_Initialize() {
    return MH_Initialize() == MH_OK;
}

export DLLEXPORT bool MHook_CreateHook(const vector<MHookConfig>& hookConfigs) {
    for (auto& config : hookConfigs) {
        auto rs = MH_CreateHook(config.pTarget, config.pDetour, config.ppOriginal);
        if (rs != MH_OK)
            return false;
    }
    return true;
}

export DLLEXPORT bool MHook_CreateHook(const vector<MHookApiConfig>& hookConfigs) {
    for (auto& config : hookConfigs) {
        auto rs = MH_CreateHookApi(config.moduleName, config.procName, config.pDetour, config.ppOriginal);
        if (rs != MH_OK)
            return false;
    }
    return true;
}

export DLLEXPORT bool MHook_EnableAll() {
    return MH_EnableHook(MH_ALL_HOOKS) == MH_OK;
}

export DLLEXPORT void MHook_Uninitialize() {
    for (auto& callback : uninitializeCallbacks())
        callback();
    MH_Uninitialize();
}
