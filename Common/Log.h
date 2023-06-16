#pragma once
#include "framework.h"
#include "macro.h"

#include "DllLoad.h"

namespace common::log {
    EXPORT_FUNC(void, Log_, OpenConsole);
    EXPORT_FUNC(void, Log_, ToFile, const char* _Format, ...);
    EXPORT_FUNC(void, Log_, HResultToFile, const char* message, HRESULT hResult);
    EXPORT_FUNC(void, Log_, LastErrorToFile, const char* message);
    EXPORT_FUNC(void, Log_, ToConsole, const char* _Format, ...);
    EXPORT_FUNC(void, Log_, FpsToConsole);
}
