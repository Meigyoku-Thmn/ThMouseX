#pragma once
#include <Windows.h>
#include <string>
#include <span>
#include <functional>

namespace common::helper::memory {
    DWORD ResolveAddress(std::span<const DWORD> offsets, bool doNotValidateLastAddress);
    std::string GetAddressConfigAsString();
    bool IsBadReadMem(const void* ptr, size_t size);
}