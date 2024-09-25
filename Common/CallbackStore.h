#pragma once
#include <Windows.h>
#include "macro.h"
#include "DataTypes.h"

namespace common::callbackstore {
    void RegisterUninitializeCallback(UninitializeCallbackType callback, bool isFromManagedCode = false);
    void RegisterPostRenderCallback(CallbackType callback);
    void RegisterClearMeasurementFlagsCallback(CallbackType callback);
    void TriggerUninitializeCallbacks(bool isProcessTerminating);
    void TriggerPostRenderCallbacks();
    void TriggerClearMeasurementFlagsCallbacks();
}