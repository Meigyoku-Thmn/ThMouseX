#pragma once
#include <Windows.h>
#include "DataTypes.h"
#include <cstdint>

namespace common::neolua {
    uintptr_t GetPositionAddress();
    void Initialize();
}