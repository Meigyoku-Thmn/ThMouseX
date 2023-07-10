#pragma once
#include "framework.h"
#include "macro.h"
#include "DataTypes.h"

#include "DllLoad.h"

namespace common::callbackstore {
    using CallbackType = void (*)(bool isProcessTerminating);
    EXPORT_FUNC(void, CallbackStore_, RegisterUninitializeCallback, CallbackType callback);
    EXPORT_FUNC(void, CallbackStore_, TriggerUninitializeCallbacks, bool isProcessTerminating);
}