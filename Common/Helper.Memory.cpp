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
        for (int i = 1; i < offsets.size(); i++) {
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
        for (int i = 0; i < g_currentConfig.Address.Length; i++) {
            auto offset = g_currentConfig.Address.Level[i];
            rs.append(format("[{:x}]", offset));
        }
        return rs;
    }

    void ScanImportTable(HMODULE hModule, ImportTableCallbackType callback) {
        auto dosHeaders = PIMAGE_DOS_HEADER(hModule);
        auto ntHeaders = PIMAGE_NT_HEADERS(DWORD_PTR(hModule) + dosHeaders->e_lfanew);
        auto importsDirectory = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
        auto importDescriptor = PIMAGE_IMPORT_DESCRIPTOR(DWORD_PTR(hModule) + importsDirectory.VirtualAddress);
        while (importDescriptor->Name != NULL) {
            auto libraryName = DWORD_PTR(hModule) + LPCSTR(importDescriptor->Name);
            callback(libraryName);
            importDescriptor++;
        }
    }
}