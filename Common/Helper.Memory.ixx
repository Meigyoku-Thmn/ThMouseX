module;

#include "framework.h"

export module common.helper.memory;

import common.var;

namespace common::helper::memory {
    export DWORD ResolveAddress(DWORD* offsets, int length) {
        if (length <= 0)
            return NULL;
        auto address = offsets[0] + (DWORD)g_targetModule;
        for (int i = 1; i < length; i++) {
            address = *PDWORD(address);
            if (address == NULL)
                break;
            address += offsets[i];
        }
        return address;
    }
}