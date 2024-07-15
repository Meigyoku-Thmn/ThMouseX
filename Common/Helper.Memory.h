#pragma once
#include "framework.h"
#include <string>
#include <span>
#include <functional>

namespace common::helper::memory {
    DWORD ResolveAddress(std::span<const DWORD> offsets);
    std::string GetAddressConfigAsString();
    void ScanImportTable(HMODULE hModule, const std::function<void(LPCSTR importDllName)> &callback);
}