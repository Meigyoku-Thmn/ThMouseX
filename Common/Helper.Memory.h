#pragma once
#include <Windows.h>
#include <string>
#include <span>
#include <functional>

namespace common::helper::memory {
    DWORD ResolveAddress(std::span<const DWORD> offsets);
    std::string GetAddressConfigAsString();
}