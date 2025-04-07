#include <Windows.h>
#include <string>
#include <format>
#include <span>
#include <vector>
#include <cstdint>

#include "Helper.Memory.h"
#include "Log.h"
#include "Variables.h"

using namespace std;

namespace note = common::log;

namespace common::helper::memory {
    static vector<DWORD> lastOffsets;
    static bool lastIsValid = true;
    uintptr_t ResolveAddress(span<const DWORD> offsets, bool doNotValidateLastAddress) {
        if (offsets.empty())
            return NULL;
        auto memInvalidated = offsets.size() != lastOffsets.size()
            || memcmp(offsets.data(), lastOffsets.data(), min(offsets.size(), lastOffsets.size()) * sizeof(offsets[0]));
        if (!memInvalidated && !lastIsValid)
            return NULL;
        if (memInvalidated) {
            lastIsValid = true;
            lastOffsets.resize(offsets.size());
            memcpy(lastOffsets.data(), offsets.data(), offsets.size() * sizeof(offsets[0]));
        }
        auto address = offsets[0] + bcast<uintptr_t>(g_targetModule);
        for (size_t i = 1; i < offsets.size(); i++) {
            if (memInvalidated && IsBadReadMem(bcast<PVOID>(address), sizeof(address))) {
                lastIsValid = false;
                note::ToFile("Access bad memory region, please check the game's version!");
                return NULL;
            }
            address = *bcast<PUINT_PTR>(address);
            if (!address)
                return NULL;
            address += offsets[i];
        }
        if (!doNotValidateLastAddress && memInvalidated && IsBadReadMem(bcast<PVOID>(address), sizeof(address))) {
            lastIsValid = false;
            note::ToFile("Access bad memory region, please check the game's version!");
            return NULL;
        }
        return address;
    }

    string GetAddressConfigAsString() {
        using enum ScriptType;
        if (g_gameConfig.ScriptType != None)
            return "Using script";
        string rs;
        rs.reserve(128);
        for (decltype(g_gameConfig.NumOfOffsets) i = 0; i < g_gameConfig.NumOfOffsets; i++) {
            auto offset = g_gameConfig.Offsets[i];
            rs.append(format("[{:x}]", offset));
        }
        return rs;
    }

    // https://gist.github.com/arbv/531040b8036050c5648fc55471d50352
    bool IsBadReadMem(LPCVOID ptr, size_t size) {
        if (size == 0)
            return false;
        if (ptr == nil)
            return true;

        auto pStart = scast<const BYTE*>(ptr);
        auto pEnd = pStart + size;

        DWORD mask = PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ
            | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;

        const BYTE* regionEnd{};
        do {
            MEMORY_BASIC_INFORMATION memInfo;
            if (VirtualQuery(pStart, &memInfo, sizeof(memInfo)) == 0)
                return true;
            else
                regionEnd = (scast<PBYTE>(memInfo.BaseAddress) + memInfo.RegionSize);
            if ((memInfo.Protect & mask) == 0 || (memInfo.Protect & (PAGE_GUARD | PAGE_NOACCESS)) != 0)
                return true;
            if (pEnd <= regionEnd)
                return false;
            else
                pStart = regionEnd;
        } while (pStart < pEnd);

        return false;
    }
}