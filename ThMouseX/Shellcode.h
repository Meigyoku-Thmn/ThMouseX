#pragma once
#include <Windows.h>
#include "../Common/DataTypes.h"

namespace core::shellcode {
    extern SIZE_T ShellcodeSectionSize;
    SHELLCODE DWORD WINAPI UnloadingShellcode(ShellcodeInput* inp);
}