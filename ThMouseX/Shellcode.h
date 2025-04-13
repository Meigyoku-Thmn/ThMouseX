#pragma once
#include <Windows.h>
#include "../Common/DataTypes.h"

namespace core::shellcode {
    extern PVOID ShellcodeFunctionPtr;
    extern SIZE_T ShellcodeSectionSize;
    void Initialize();
}