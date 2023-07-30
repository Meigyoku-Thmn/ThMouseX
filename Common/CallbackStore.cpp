#include "framework.h"
#include "macro.h"
#include "DataTypes.h"
#include "Variables.h"
#include <vector>
#include "CallbackStore.h"

using namespace std;

namespace common::callbackstore {
    struct UninitializeCallbackItem {
        UninitializeCallbackType callback;
        bool isFromDotNet;
    };
    vector<UninitializeCallbackItem> uninitializeCallbacks;
    vector<CallbackType> postRenderCallbacks;
    vector<CallbackType> clearMeasurementFlagsCallbacks;

    void RegisterUninitializeCallback(UninitializeCallbackType callback, bool isFromDotNet) {
        uninitializeCallbacks.emplace_back(callback, isFromDotNet);
    }
    void RegisterPostRenderCallback(CallbackType callback) {
        postRenderCallbacks.push_back(callback);
    }
    void RegisterClearMeasurementFlagsCallback(CallbackType callback) {
        clearMeasurementFlagsCallbacks.push_back(callback);
    }

    void TriggerUninitializeCallbacks(bool isProcessTerminating) {
        for (auto& item : uninitializeCallbacks) {
            if (isProcessTerminating && item.isFromDotNet)
                continue;
            item.callback(isProcessTerminating);
        }
    }
    void TriggerPostRenderCallbacks() {
        for (auto& callback : postRenderCallbacks)
            callback();
    }
    void TriggerClearMeasurementFlagsCallbacks() {
        for (auto& callback : clearMeasurementFlagsCallbacks)
            callback();
    }
}