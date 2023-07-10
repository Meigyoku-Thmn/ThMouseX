#include "framework.h"
#include "macro.h"
#include "DataTypes.h"
#include "Variables.h"
#include <vector>
#include "CallbackStore.h"

using namespace std;

namespace common::callbackstore {
    vector<UninitializeCallbackType>& uninitializeCallbacks() {
        static vector<UninitializeCallbackType> backing;
        return backing;
    }
    vector<CallbackType>& postRenderCallbacks() {
        static vector<CallbackType> backing;
        return backing;
    }
    vector<CallbackType>& clearMeasurementFlagsCallbacks() {
        static vector<CallbackType> backing;
        return backing;
    }

    void RegisterUninitializeCallback(UninitializeCallbackType callback) {
        uninitializeCallbacks().push_back(callback);
    }
    void RegisterPostRenderCallback(CallbackType callback) {
        postRenderCallbacks().push_back(callback);
    }
    void RegisterClearMeasurementFlagsCallback(CallbackType callback) {
        clearMeasurementFlagsCallbacks().push_back(callback);
    }

    void TriggerUninitializeCallbacks(bool isProcessTerminating) {
        for (auto& callback : uninitializeCallbacks()) callback(isProcessTerminating);
    }
    void TriggerPostRenderCallbacks() {
        for (auto& callback : postRenderCallbacks()) callback();
    }
    void TriggerClearMeasurementFlagsCallbacks() {
        for (auto& callback : clearMeasurementFlagsCallbacks()) callback();
    }
}