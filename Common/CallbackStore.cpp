#include "framework.h"
#include "macro.h"
#include "DataTypes.h"
#include "Variables.h"
#include <vector>
#include "CallbackStore.h"

using namespace std;

namespace common::callbackstore {
    vector<CallbackType>& uninitializeCallbacks() {
        static vector<CallbackType> backing;
        return backing;
    }

    void RegisterUninitializeCallback(CallbackType callback) {
        uninitializeCallbacks().push_back(callback);
    }

    void TriggerUninitializeCallbacks(bool isProcessTerminating) {
        for (auto& callback : uninitializeCallbacks())
            callback(isProcessTerminating);
    }
}