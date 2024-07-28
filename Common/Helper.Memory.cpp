#include "framework.h"
#include <string>
#include <format>
#include <span>

#include "Helper.Memory.h"
#include "Variables.h"

using namespace std;

namespace common::helper::memory {
    DWORD ResolveAddress(span<const DWORD> offsets) {
        if (offsets.size() <= 0)
            return NULL;
        auto address = offsets[0] + DWORD(g_targetModule);
        for (size_t i = 1; i < offsets.size(); i++) {
            address = *PDWORD(address);
            if (!address)
                break;
            address += offsets[i];
        }
        return address;
    }

    string GetAddressConfigAsString() {
        if (g_currentConfig.ScriptType != ScriptType::None)
            return "Using script";
        string rs;
        rs.reserve(128);
        for (size_t i = 0; i < g_currentConfig.Address.Length; i++) {
            auto offset = g_currentConfig.Address.Level[i];
            rs.append(format("[{:x}]", offset));
        }
        return rs;
    }
}