#pragma once
#include "framework.h"
#include "macro.h"
#include "DataTypes.h"

namespace common::callbackstore {
    using CallbackType = void (*)(bool isProcessTerminating);
    void RegisterUninitializeCallback(CallbackType callback);
    void TriggerUninitializeCallbacks(bool isProcessTerminating);
}