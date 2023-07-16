#pragma once
#include "framework.h"
#include "macro.h"

namespace common::log {
    void OpenConsole();
    void ToFile(const char* _Format, ...);
    void DxErrToFile(const char* message, HRESULT hResult);
    void HResultToFile(const char* message, HRESULT hResult);
    void LastErrorToFile(const char* message);
    void ToConsole(const char* _Format, ...);
    void FpsToConsole();
}
