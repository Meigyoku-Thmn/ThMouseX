#pragma once
#include "framework.h"

namespace common::helper::memory {
    using ImportTableCallbackType = void (*)(LPCSTR importDllName);
    DWORD ResolveAddress(DWORD* offsets, int length);
    void ScanImportTable(HMODULE hModule, ImportTableCallbackType callback);
}