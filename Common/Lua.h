#pragma once
#include <Windows.h>
#include <cstdint>

namespace common::lua {
    void Initialize();
    uintptr_t GetPositionAddress();
}
