#pragma once
#include "framework.h"
#include "macro.h"
#include "DataTypes.h"

namespace common::callbackstore {
    using UninitializeCallbackType = void (*)(bool isProcessTerminating);
    using CallbackType = void (*)(void);
    void RegisterUninitializeCallback(UninitializeCallbackType callback);
    void RegisterPostRenderCallback(CallbackType callback);
    void RegisterClearMeasurementFlagsCallback(CallbackType callback);
    void TriggerUninitializeCallbacks(bool isProcessTerminating);
    void TriggerPostRenderCallbacks();
    void TriggerClearMeasurementFlagsCallbacks();
}