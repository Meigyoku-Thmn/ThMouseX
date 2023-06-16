#pragma once
#include "framework.h"
#include "macro.h"

#ifndef COMMON_EXPORTS
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <string>
namespace common::dll {
    using namespace std;
    inline WCHAR currentModuleDirPath[MAX_PATH + 1];
    inline HMODULE hMod;
    inline FARPROC ImportFunction(const char* functionName) {
        if (hMod == NULL) {
            auto modulePath = wstring(currentModuleDirPath) + L"\\" + L"Common.dll";
            hMod = LoadLibraryExW(modulePath.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
        }
        if (hMod == NULL)
            return NULL;
        return GetProcAddress(hMod, functionName);
    }
    inline BOOL Unload() {
        auto rs = FreeLibrary(hMod);
        hMod = NULL;
        return rs;
    }
    inline struct OnInit_RunTimeDllLoad {
        OnInit_RunTimeDllLoad() {
            HMODULE hmod;
            auto dwFlags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
            GetModuleHandleExW(dwFlags, (LPWSTR)&ImportFunction, &hmod);
            GetModuleFileNameW(hmod, currentModuleDirPath, ARRAYSIZE(currentModuleDirPath));
            currentModuleDirPath[ARRAYSIZE(currentModuleDirPath) - 1] = '\0';
            PathRemoveFileSpecW(currentModuleDirPath);
        }
        ~OnInit_RunTimeDllLoad() {
            Unload();
        }
    } _;
}
#endif

#ifdef COMMON_EXPORTS
#define EXPORT_FUNC(retType, prefix, funcName, ...) \
retType funcName(__VA_ARGS__); \
DLLEXPORT_C inline decltype(&funcName) GetFunc_##prefix##funcName() { \
    return &funcName; \
}
#else
#define EXPORT_FUNC(retType, prefix, funcName, ...) \
retType funcName##_Sig(__VA_ARGS__); \
decltype(&funcName##_Sig) GetFunc_##prefix##funcName(); \
inline decltype(&funcName##_Sig) funcName = ((decltype(&GetFunc_##prefix##funcName))common::dll::ImportFunction(QUOTE(GetFunc_##prefix##funcName)))();
#endif

#ifdef COMMON_EXPORTS
#define EXPORT_VAR(type, varName, ...) \
extern type varName __VA_ARGS__; \
DLLEXPORT_C inline type (&GetVar_##varName())__VA_ARGS__ { \
    return varName; \
}
#else
#define EXPORT_VAR(type, varName, ...) \
type (&GetVar_##varName())__VA_ARGS__; \
inline type (&varName)__VA_ARGS__ = ((decltype(&GetVar_##varName))common::dll::ImportFunction(QUOTE(GetVar_##varName)))();
#endif