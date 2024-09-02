#pragma once
#include <Windows.h>
#include <vector>

#include "../Common/MinHook.h"
#include "../Common/macro.h"

namespace core::messagequeue {
    DLLEXPORT_C bool InstallHooks();
    DLLEXPORT_C void RemoveHooks();
    void Initialize();
}