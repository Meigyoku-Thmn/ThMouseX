#pragma once
#include "framework.h"
#include <string>

namespace common::helper::memory {
    using ImportTableCallbackType = void (*)(LPCSTR importDllName);
    DWORD ResolveAddress(DWORD* offsets, int length);
    std::string GetAddressConfigAsString();
    void ScanImportTable(HMODULE hModule, ImportTableCallbackType callback);
}