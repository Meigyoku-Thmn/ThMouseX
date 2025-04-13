#pragma once
#ifndef _WIN64
#include <Windows.h>
#include <vector>

#include "../Common/MinHook.h"

namespace core::directx8 {
    void Initialize();
}
#endif