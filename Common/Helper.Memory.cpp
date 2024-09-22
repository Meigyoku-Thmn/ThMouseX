#include <Windows.h>
#include <string>
#include <format>
#include <span>
#include <vector>

#include "Helper.Memory.h"
#include "Log.h"
#include "Variables.h"

using namespace std;

namespace note = common::log;

namespace common::helper::memory {
    thread_local vector<DWORD> lastOffsets;
    thread_local bool lastIsValid = true;
    DWORD ResolveAddress(span<const DWORD> offsets) {
        if (offsets.size() <= 0)
            return NULL;
        auto memCheck = offsets.size() != lastOffsets.size()
            || memcmp(offsets.data(), lastOffsets.data(), min(offsets.size(), lastOffsets.size()) * sizeof(offsets[0]));
        if (!memCheck && !lastIsValid)
            return NULL;
        if (memCheck) {
            lastIsValid = true;
            lastOffsets.resize(offsets.size());
            memcpy(lastOffsets.data(), offsets.data(), offsets.size() * sizeof(offsets[0]));
        }
        auto address = offsets[0] + DWORD(g_targetModule);
        for (size_t i = 1; i < offsets.size(); i++) {
            if (memCheck && IsBadReadMem((PVOID)address, sizeof(address))) {
                lastIsValid = false;
                note::ToFile("Access bad memory region, please check the game's version!");
                return NULL;
            }
            address = *PDWORD(address);
            if (!address)
                break;
            address += offsets[i];
        }
        if (memCheck && IsBadReadMem((PVOID)address, sizeof(address))) {
            lastIsValid = false;
            note::ToFile("Access bad memory region, please check the game's version!");
            return NULL;
        }
        return address;
    }

    string GetAddressConfigAsString() {
        if (g_gameConfig.ScriptType != ScriptType_None)
            return "Using script";
        string rs;
        rs.reserve(128);
        auto& addressChain = g_gameConfig.AddressChain;
        auto lowerBound = addressChain.GetLowerBound();
        auto upperBound = addressChain.GetUpperBound();
        for (auto i = lowerBound; i <= upperBound; i++) {
            auto offset = addressChain[i];
            rs.append(format("[{:x}]", offset));
        }
        return rs;
    }

    // https://gist.github.com/arbv/531040b8036050c5648fc55471d50352
    bool IsBadReadMem(const void* ptr, size_t size) {
        if (size == 0)
            return false;
        if (ptr == nullptr)
            return true;

        auto pStart = (PBYTE)ptr;
        auto pEnd = pStart + size;

        DWORD mask = PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ
            | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;

        PBYTE regionEnd{};
        do {
            MEMORY_BASIC_INFORMATION memInfo;
            if (pStart == ptr || pStart == regionEnd) {
                if (VirtualQuery(pStart, &memInfo, sizeof(memInfo)) == 0)
                    return true;
                else
                    regionEnd = ((PBYTE)memInfo.BaseAddress + memInfo.RegionSize);
            }
            if ((memInfo.Protect & mask) == 0 || (memInfo.Protect & (PAGE_GUARD | PAGE_NOACCESS)) != 0)
                return true;
            if (pEnd <= regionEnd)
                return false;
            else if (pEnd > regionEnd)
                pStart = regionEnd;
        } while (pStart < pEnd);

        return false;
    }
}