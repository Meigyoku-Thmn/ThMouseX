#include <Windows.h>
#include "macro.h"
#include "DataTypes.h"
#include "Variables.h"
#include <vector>
#include "CallbackStore.h"

using namespace std;

namespace common::callbackstore {
    vector<UninitializeCallbackType> uninitializeCallbacks;
    vector<UninitializeCallbackType> uninitializeManagedCallbacks;
    vector<CallbackType> postRenderCallbacks;
    vector<CallbackType> clearMeasurementFlagsCallbacks;

    void RegisterUninitializeCallback(UninitializeCallbackType callback, bool isFromManagedCode) {
        if (isFromManagedCode)
            uninitializeManagedCallbacks.push_back(callback);
        else
            uninitializeCallbacks.push_back(callback);
    }
    void RegisterPostRenderCallback(CallbackType callback) {
        postRenderCallbacks.push_back(callback);
    }
    void RegisterClearMeasurementFlagsCallback(CallbackType callback) {
        clearMeasurementFlagsCallbacks.push_back(callback);
    }

    void TriggerUninitializeCallbacks(bool isProcessTerminating) {
        for (auto const& callback : uninitializeManagedCallbacks)
            callback(isProcessTerminating);
        for (auto const& callback : uninitializeCallbacks)
            callback(isProcessTerminating);
    }
    void TriggerPostRenderCallbacks() {
        for (auto const& callback : postRenderCallbacks)
            callback();
    }
    void TriggerClearMeasurementFlagsCallbacks() {
        for (auto& callback : clearMeasurementFlagsCallbacks)
            callback();
    }
}