#pragma once
#include "framework.h"
#include <string>
#include <span>
#include <functional>

namespace common::helper::memory {
    using ImportTableCallbackType = void (*)(LPCSTR importDllName);
    DWORD ResolveAddress(std::span<const DWORD> offsets);
    std::string GetAddressConfigAsString();
    void ScanImportTable(HMODULE hModule, ImportTableCallbackType callback);
}