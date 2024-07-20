#include "framework.h"
#include "Initialization.h"
#include "../Common/Variables.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            g_coreModule = hModule;
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            core::Uninitialize(lpReserved != null);
            break;
    }
    return TRUE;
}

