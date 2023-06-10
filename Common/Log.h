#pragma once
#include "framework.h"
#include "macro.h"

namespace common::log {
    DLLEXPORT void OpenConsole();
    DLLEXPORT void ToFile(const char* _Format, ...);
    DLLEXPORT void HResultToFile(const char* message, HRESULT hResult);
    DLLEXPORT void LastErrorToFile(const char* message);
    DLLEXPORT void ToConsole(const char* _Format, ...);
    DLLEXPORT void FpsToConsole();
}
