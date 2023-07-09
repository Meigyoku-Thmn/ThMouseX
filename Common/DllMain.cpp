#include "framework.h"
#include <clocale>
#include <shlwapi.h>

#include "Variables.h"

using namespace std;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH: {

            setlocale(LC_ALL, ".UTF8");
            setlocale(LC_NUMERIC, "C");
            DisableThreadLibraryCalls(hModule);

            g_commonModule = hModule;

            GetModuleFileNameW(g_commonModule, g_currentModuleDirPath, ARRAYSIZE(g_currentModuleDirPath));
            g_currentModuleDirPath[ARRAYSIZE(g_currentModuleDirPath) - 1] = '\0';
            PathRemoveFileSpecW(g_currentModuleDirPath);

            break;
        }
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

