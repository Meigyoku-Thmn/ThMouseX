#pragma once
#include <cstdint>
#include <Windows.h>

namespace common::luajit {
    void Initialize();
    uintptr_t GetPositionAddress();
}
