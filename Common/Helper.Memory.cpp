#include "framework.h"

#include "Helper.Memory.h"
#include "Variables.h"

namespace common::helper::memory {
    DWORD ResolveAddress(DWORD* offsets, int length) {
        if (length <= 0)
            return NULL;
        auto address = offsets[0] + (DWORD)g_targetModule;
        for (int i = 1; i < length; i++) {
            address = *PDWORD(address);
            if (!address)
                break;
            address += offsets[i];
        }
        return address;
    }
}