#include "framework.h"
#include <clocale>
#include <shlwapi.h>
#include <string>
#include <delayimp.h>

using namespace std;

WCHAR currentModuleDirPath[MAX_PATH + 1];
WCHAR lastDirPath[MAX_PATH + 1];

void LoadCurrentModuleDirPath() {
    if (currentModuleDirPath[0] != '\0')
        return;
    HMODULE hmod;
    auto dwFlags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
    GetModuleHandleExW(dwFlags, (LPWSTR)&LoadCurrentModuleDirPath, &hmod);
    GetModuleFileNameW(hmod, currentModuleDirPath, ARRAYSIZE(currentModuleDirPath));
    currentModuleDirPath[ARRAYSIZE(currentModuleDirPath) - 1] = '\0';
    PathRemoveFileSpecW(currentModuleDirPath);
}

HMODULE TryLoadInMemoryModule(const char* moduleName) {
    auto modulePath = wstring(currentModuleDirPath) + L"\\" + wstring(moduleName, moduleName + strlen(moduleName));
    return GetModuleHandleW(modulePath.c_str());
}

FARPROC WINAPI delayHook(unsigned dliNotify, PDelayLoadInfo pdli)
{
    LoadCurrentModuleDirPath();
    if (dliNotify == dliNotePreLoadLibrary) {
        auto hMod = TryLoadInMemoryModule(pdli->szDll);
        if (hMod != NULL)
            return (FARPROC)hMod;
        GetCurrentDirectoryW(ARRAYSIZE(lastDirPath), lastDirPath);
        SetCurrentDirectoryW(currentModuleDirPath);
    }
    else {
        if (lastDirPath[0] != '\0')
            SetCurrentDirectoryW(lastDirPath);
        lastDirPath[0] = '\0';
    }
    return NULL;
}
ExternC PfnDliHook __pfnDliNotifyHook2 = delayHook;
ExternC PfnDliHook __pfnDliFailureHook2 = delayHook;

void RemoveDelayHook() {
    __pfnDliNotifyHook2 = NULL;
    __pfnDliFailureHook2 = NULL;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            RemoveDelayHook();

            setlocale(LC_ALL, ".UTF8");
            setlocale(LC_NUMERIC, "C");

            LoadCurrentModuleDirPath();

            DisableThreadLibraryCalls(hModule);
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

